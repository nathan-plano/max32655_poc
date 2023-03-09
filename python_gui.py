from tkinter import * 
from matplotlib.figure import Figure
import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, ifft
from random import randint 
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg 
import time 
import threading 
import asyncio
import logging
from bleak import discover
from bleak import BleakClient
from scipy import signal
from datetime import datetime
import os



## global varibles
continuePlotting = False 
FFT_Time_state = 1
global data_len #=2000
global time_steps #= np.arange(0,data_len/Sample_Rate,1/Sample_Rate)
global freq_steps #= np.arange(0,Sample_Rate/2,Sample_Rate/data_len)
global signal 
ready=True 
lock = threading.Lock()
global check 
global start_sending
global address
global connected
global collect_data_flag
global send_data_len

#Scans bluetooth devices nearby
#input- 
#name= name of device peripheral to connect to 
#ouput- 
# The address assoicated with the bluetooth devivce with name as its name
async def scan(name):
    dev = await discover()
    
    for i in range(0,len(dev)):
        #Print the devices discovered
        print("[" + str(i) + "]" + dev[i].address,dev[i].name,dev[i].metadata["uuids"])
        if(dev[i].name==name):
            return dev[i].address
        

#notification_handler handler, happens when data is set from ble to the pc running this code,
def notification_handler(sender, data):

    #global variables
    global check
    global signal
    global start_sending
    global send_data_len
    global data_len
    number_chuncks = int(3*data_len/250)
    


    # if length is one this is a notifaction to the pc that 
    # the ble has data it wants to send, this must happen first before
    # data can be sent, it sets the start_sending global to 1 
    if (len(data)==1):
        start_sending=1
        print("notifying")
    
    elif(len(data)==2):
        send_data_len=1
        print("asking For data len")
    
    # else you will be getting data packets of size 501 bytes
    else:
        # first byte is the index value, aka where to put the 500 bytes in relation to the numpy array 
        start= data[0]
        #print("notify")

        # if start ==12 transaction has ended only 12 500 byte chuncks are sent from ble to pc during a transaction 
        if(start==number_chuncks):
            print("finished")
        
        #if the check flag for this data has not been sent store the 500 bytes into the numpy array 
        #starting at the start index where start is increments of 250 positions in the numpy array 
        # as each postion is a 16 bit number but we can only send bytes
        else:
            if(check[start]==0):
                for i in range(int(len(data)/2)):
                    index = i*2+1
                    byte_val =data[index:index+2]
                    value =int.from_bytes(byte_val, "little", signed=True)
                    # if(value>30000 or value<-30000):
                    #     value=0
                    signal[start*250+i]=value
                print(signal[start*250:start*250+100])

            check[start]=1
   


# this function is in charge of connecting this python code to the ble and creating the transactions 
# the address is the addres of the ble to connect to 
async def run(address):
    global lock 
    global check
    global start_sending
    global connected
    global start_sending
    global send_data_len
    global data_len
    start_sending=0

    global collect_data_flag
    number_chuncks = int(3*data_len/250)


    async with BleakClient(address) as client:
        #connect to client 
        x = await client.is_connected()

        #you may have to change this value to your UUID for your device
        uuid = 'E0262760-08C2-11E1-9073-0E8AC72E0001'

        # start notfication handler for the specific UUID above
        await client.start_notify(uuid, notification_handler)  

        # conncected global flag means that the pc and the ble are connected
        connected =1
        #while the they are conneted stay in this loop
        while(connected): 
           # print("stuck333333")           
            lock.acquire()
            print("updater got lock")
            if(send_data_len==1):
                print(number_chuncks)
                
                await client.write_gatt_char(uuid,bytearray([0xFF,0xFF,number_chuncks]))
                print(number_chuncks)
                send_data_len=0

            elif(start_sending==1):
                check =[0] * number_chuncks
                check_sent =[0] * number_chuncks
                for i in range(number_chuncks+1):
                   # print(check)
                    if(i==number_chuncks):
                        await client.write_gatt_char(uuid,bytearray([i]))
                    else:                                 
                        while(check_sent[i]==0 ):
                            check_sent[i]=1 
                            check[i] =0
                            
                            await client.write_gatt_char(uuid,bytearray([i]))
                            num=0 
                            while(check[i]==0 ):
                                num= num+1
                                await asyncio.sleep(0.01)
                                if (num>10):
                                    check_all_sent=0
                                    check[i] =1
                                    check_sent[i]=0
                if(collect_data_flag==1):
                    save_data()
                start_sending=0
            else:
                await asyncio.sleep(0.01)
            
            
            lock.release()
            print("updater realsed lock")
        await client.stop_notify(uuid)
    await client.disconnect()
        

# this function is called when the connect to ble button on the gui is called 
def connect():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    address= loop.run_until_complete(scan("Periph"))
    print("Address is " + address)
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(address))
    print("done")



# Changes global state of continues plotting 
def change_state(): 
    global continuePlotting 
    if continuePlotting == True: 
        continuePlotting = False 
    else: 
        continuePlotting = True 

# gets the data this will change as it is dummy data the momment
def get_data():
    global data_len
    global signal   
    vibx_signal =signal[0:data_len]
    vibz_signal = signal[data_len:data_len*2]
    flux_signal = signal[data_len*2:data_len*3]
    CNN_ouput = np.round(np.random.rand(1))
    if FFT_Time_state==1:
        vibx_signal = abs(fft(vibx_signal))[0:int(data_len/2)]
        vibz_signal = abs(fft(vibz_signal))[0:int(data_len/2)]
        flux_signal = abs(fft(flux_signal))[0:int(data_len/2)]
        vibx_signal[0] = 0
        vibz_signal[0] = 0
        flux_signal[0] = 0
    return vibx_signal,vibz_signal,flux_signal,CNN_ouput

def save_data():
    global signal
    global data_len
    global directory
    global total_num_windows
    global current_num_windows
    global collect_data_flag

    if(current_num_windows<total_num_windows):
        current_num_windows=current_num_windows+1
        save = np.empty((1,3,data_len))
        vibx_signal =signal[0:data_len]
        vibz_signal = signal[data_len:2*data_len]
        flux_signal = signal[2*data_len:3*data_len]
        save[0,0]=vibx_signal
        save[0,1]=vibz_signal
        save[0,2]=flux_signal
        now = datetime.now()
        date_time = now.strftime("%m_%d_%Y_%H_%M_%S")
        my_file = directory+"/"+date_time+'npy'
        np.save(my_file, save)
    else:
        collect_data_flag=0
    return
    



def app(): 

    global continuePlotting 
    global collect_data_flag
    
    continuePlotting=True
    collect_data_flag=0
    
    root = Tk() 
    root.geometry("5000x2000") 
    fig, ax = plt.subplots(3) 
    graph = FigureCanvasTkAgg(fig, master=root) 
    graph.get_tk_widget().pack(side="bottom",fill='both',expand=True) 
    c= Canvas( width = 600, height = 200, bg = "black")
    oval =c.create_oval(10, 10, 590,190 , fill="green")
    text = c.create_text(300, 100, text="Healthy Data", fill="black", font=('Helvetica 30 bold'))
    c.pack()
    c.place(x=500, y=0)
    
 
    def plotter(): 
         
        while continuePlotting: 

            global FFT_Time_state
            global lock 
            global data_len
            Sample_Rate =4000
            time_steps = np.arange(0,data_len/Sample_Rate,1/Sample_Rate)
            freq_steps = np.arange(0,Sample_Rate/2,Sample_Rate/data_len)
            
            lock.acquire()
            print("plotter got lock ")
            dpts = get_data()
            print("0")
            
            ax[0].cla()

            ax[1].cla()
            ax[2].cla()
            print("1")
  
            if (FFT_Time_state==0):
                ax[0].plot(time_steps,dpts[0])
                ax[1].plot(time_steps,dpts[1])
                ax[2].plot(time_steps,dpts[2])
                ax[0].set_title ('Vibx Time Series Plot',fontsize=22)
                ax[1].set_title ('Viby Time Series Plot',fontsize=22)
                ax[2].set_title ('Vibz Time Series Plot',fontsize=22)
                ax[0].set_xlabel('Time',fontsize=15)
                ax[1].set_xlabel('Time',fontsize=15)
                ax[2].set_xlabel('Time',fontsize=15)
            else:
                ax[0].plot(freq_steps,dpts[0])
                ax[1].plot(freq_steps,dpts[1])
                ax[2].plot(freq_steps,dpts[2])
                ax[0].set_title ('Vibx Frequnecy Spectrum ',fontsize=22)
                ax[1].set_title ('Viby Frequnecy Spectrum ',fontsize=22)
                ax[2].set_title ('Vibz Frequnecy Spectrum ',fontsize=22)
                ax[0].set_xlabel('Frequnecy',fontsize=15)
                ax[1].set_xlabel('Frequnecy',fontsize=15)
                ax[2].set_xlabel('Frequnecy',fontsize=15)
        
            graph.draw()
            if(dpts[3]==0):
                c.itemconfig(oval, fill='green')
                c.itemconfig(text, text="Healthy Data")
            else:
                c.itemconfig(oval, fill='red')
                c.itemconfig(text, text="Anomalous Data")
            lock.release()
            
            print("plotter lost lock ") 
            time.sleep(0.1) 
 

    def gui_handler(): 
        print("test")
        change_state() 
        threading.Thread(target=plotter, daemon=True).start()
    def connect_handler(): 
        global data_len
        global signal 
        data_len=int(E3.get())
        signal= np.zeros(3*data_len) 
        threading.Thread(target=connect, daemon=True).start() 
    def disconnect_handler():
        global connected
        connected=0
    def leave():
        quit()

    def fft_time_change():
        global FFT_Time_state 
        if (FFT_Time_state==0):
            FFT_Time_state=1
        else:
            FFT_Time_state=0
    
    def collect_data():
        global directory
        global total_num_windows
        global current_num_windows
        global collect_data_flag
        current_num_windows=0
        total_num_windows=int(E1.get())
        directory = E2.get()
        collect_data_flag=1

    b = Button(root, text="Start/Stop", command=gui_handler, bg="green", fg="white",height= 4, width=15,font=('Helvetica bold', 26)) 
    b.pack() 
    b.place(x=100, y=0)

    leave = Button(root, text="Exit", command=leave, bg="red", fg="white",height= 4, width=15,font=('Helvetica bold', 26)) 
    leave.pack() 
    leave.place(x=100, y=200)

    fft_time_button = Button(root, text="FFT/Time Switch", command=fft_time_change, bg="orange", fg="white",height= 4, width=15,font=('Helvetica bold', 26)) 
    fft_time_button.pack() 
    fft_time_button.place(x=100, y=400)

    connect_button = Button(root, text="Connect to BLE", command=connect_handler, bg="blue", fg="white",height= 4, width=15,font=('Helvetica bold', 26)) 
    connect_button.pack() 
    connect_button.place(x=100, y=600)

    disconnect_button = Button(root, text="Disconnect BLE", command=disconnect_handler, bg="purple", fg="white",height= 4, width=15,font=('Helvetica bold', 26)) 
    disconnect_button.pack() 
    disconnect_button.place(x=100, y=800)

    collect_data_button = Button(root, text="Collect Data ", command=collect_data, bg="pink", fg="white",height= 4, width=15,font=('Helvetica bold', 26)) 
    collect_data_button.pack() 
    collect_data_button.place(x=100, y=1220)

    L1 = Label(root, text="Number of Windows To Collect", font=('Arial 24'))
    L1.pack( )
    L1.place(x=10, y=1000)
    E1 = Entry(root, bd =10, font=('Arial 24'))
    E1.pack()
    E1.place(x=10, y=1050)
    L2 = Label(root, text="Data Directory Location", font=('Arial 24'))
    L2.pack( )
    L2.place(x=10, y=1100)
    E2 = Entry(root, bd =10, font=('Arial 24'))
    E2.pack()
    E2.place(x=10, y=1150)

    L3 = Label(root, text="Length of Data", font=('Arial 24'))
    L3.pack( )
    L3.place(x=10, y=1420)
    E3 = Entry(root, bd =10, font=('Arial 24'))
    E3.pack()
    E3.place(x=10, y=1470)




    root.mainloop() 
 
if __name__ == '__main__': 
    app() 

