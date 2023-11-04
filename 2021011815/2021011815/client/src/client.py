import socket
import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from tkinter import messagebox
from tkinter import simpledialog
from threading import Thread
import sys
import time
import re
import os
import argparse

class CustomStringDialog(simpledialog.Dialog):
    def body(self, master):
        self.entry = tk.Entry(master)
        self.entry.pack()
        return self.entry

    def apply(self):
        self.result = self.entry.get()


# ftpclientgui
class ClientGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("FTP Client")

        self.server_frame = tk.Frame(root)
        self.server_frame.pack(side=tk.TOP)

        self.server_ip_label = tk.Label(self.server_frame, text="Server IP")
        self.server_ip_label.pack(side=tk.LEFT)
        self.server_ip_entry = tk.Entry(self.server_frame)
        self.server_ip_entry.pack(side=tk.LEFT)
        self.server_port_label = tk.Label(self.server_frame, text="Server Port")
        self.server_port_label.pack(side=tk.LEFT)
        self.server_port_entry = tk.Entry(self.server_frame)
        self.server_port_entry.pack(side=tk.LEFT)
        self.username_label = tk.Label(self.server_frame, text="Username")
        self.username_label.pack(side=tk.LEFT)
        self.username_entry = tk.Entry(self.server_frame)
        self.username_entry.pack(side=tk.LEFT)
        self.password_label = tk.Label(self.server_frame, text="Password")
        self.password_label.pack(side=tk.LEFT)
        self.password_entry = tk.Entry(self.server_frame, show="*")
        self.password_entry.pack(side=tk.LEFT)

        self.connect_button = tk.Button(self.server_frame, text="Connect", command=self.connect)
        self.connect_button.pack(side=tk.LEFT)

        
        # message and status
        self.message_status_frame = tk.Frame(self.root)
        self.message_status_frame.pack(fill=tk.BOTH, expand=True)
        self.message_frame = tk.Frame(self.message_status_frame)
        self.message_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.message_response_text = tk.Text(self.message_frame)
        self.message_response_text.pack(fill=tk.BOTH, expand=True)

        # status
        # self.data_mode_frame = tk.Frame(self.message_status_frame)
        # self.data_mode_frame.pack(side=tk.LEFT, fill=tk.Y)
        # self.data_mode_line_1_frame = tk.Frame(self.data_mode_frame)
        # self.data_mode_line_1_frame.pack(side=tk.LEFT)
        # self.data_mode_line_2_frame = tk.Frame(self.data_mode_frame)
        # self.data_mode_line_2_frame.pack(side=tk.LEFT)

        # self.data_mode_status_label = tk.Label(self.data_mode_line_1_frame, text="Mode")
        # self.data_mode_status_label.pack(side=tk.LEFT)
        # self.data_mode_status_text = tk.Text(self.data_mode_line_1_frame, height=1, width=5)
        # self.data_mode_status_text.pack(side=tk.LEFT)
        # self.port_button = tk.Button(self.data_mode_line_2_frame, text="PORT", command=self.handle_port_button)
        # self.port_button.pack(side=tk.LEFT)
        # self.pasv_button = tk.Button(self.data_mode_line_2_frame, text="PASV", command=lambda:self.client.handle_msg('PASV'))
        # self.pasv_button.pack(side=tk.LEFT)
        # self.data_mode_status = tk.Label(self.data_mode_frame, text="None")
        # self.data_mode_status.pack(side=tk.LEFT)
        self.data_mode_frame = tk.Frame(self.message_status_frame)
        self.data_mode_frame.pack(side=tk.LEFT, fill=tk.Y)

        # line 1
        self.data_mode_line_1_frame = tk.Frame(self.data_mode_frame)
        self.data_mode_line_1_frame.grid(row=0, column=0, pady=10)
        self.data_mode_status_label = tk.Label(self.data_mode_line_1_frame, text="Mode")
        self.data_mode_status_label.pack(side=tk.LEFT)
        self.data_mode_status_text = tk.Text(self.data_mode_line_1_frame, height=1, width=5)
        self.data_mode_status_text.pack(side=tk.LEFT)

        # line 2
        self.data_mode_line_2_frame = tk.Frame(self.data_mode_frame)
        self.data_mode_line_2_frame.grid(row=1, column=0, pady=10)
        self.port_button = tk.Button(self.data_mode_line_2_frame, text="PORT", command=self.handle_port_button)
        self.port_button.pack(side=tk.LEFT)
        self.pasv_button = tk.Button(self.data_mode_line_2_frame, text="PASV", command=lambda:self.client.handle_msg('PASV'))
        self.pasv_button.pack(side=tk.LEFT)

        # line 3
        self.data_mode_line_3_frame = tk.Frame(self.data_mode_frame)
        self.data_mode_line_3_frame.grid(row=2, column=0, pady=10)
        self.retr_button = tk.Button(self.data_mode_line_3_frame, text="RETR", command=self.handle_retr_button)
        self.retr_button.pack(side=tk.LEFT)
        self.stor_button = tk.Button(self.data_mode_line_3_frame, text="STOR", command=self.handle_stor_button)
        self.stor_button.pack(side=tk.LEFT)

        # line 4
        self.data_mode_line_4_frame = tk.Frame(self.data_mode_frame)
        self.data_mode_line_4_frame.grid(row=3, column=0, pady=10)
        self.mkd_button = tk.Button(self.data_mode_line_4_frame, text="MKD", command=self.handle_mkd_button)
        self.mkd_button.pack(side=tk.LEFT)
        self.rmd_button = tk.Button(self.data_mode_line_4_frame, text="RMD", command=self.handle_rmd_button)
        self.rmd_button.pack(side=tk.LEFT)

        # line 5
        self.data_mode_line_5_frame = tk.Frame(self.data_mode_frame)
        self.data_mode_line_5_frame.grid(row=4, column=0, pady=10)
        self.cwd_button = tk.Button(self.data_mode_line_5_frame, text="CWD", command=self.handle_cwd_button)
        self.cwd_button.pack(side=tk.LEFT)
        self.pwd_button = tk.Button(self.data_mode_line_5_frame, text="PWD", command=lambda:self.client.handle_msg('PWD'))
        self.pwd_button.pack(side=tk.LEFT)

        # line 6
        self.data_mode_line_6_frame = tk.Frame(self.data_mode_frame)
        self.data_mode_line_6_frame.grid(row=5, column=0, pady=10)
        self.type_button = tk.Button(self.data_mode_line_6_frame, text="TYPE", command=lambda:self.client.handle_msg('TYPE I'))
        self.type_button.pack(side=tk.LEFT)
        self.syst_button = tk.Button(self.data_mode_line_6_frame, text="SYST", command=lambda:self.client.handle_msg('SYST'))
        self.syst_button.pack(side=tk.LEFT)



        self.data_mode_line_7_frame = tk.Frame(self.data_mode_frame)
        self.data_mode_line_7_frame.grid(row=6, column=0, pady=10)
        self.list_button = tk.Button(self.data_mode_line_7_frame, text="LIST", command=self.handle_list_button)
        self.list_button.pack(side=tk.LEFT)
        self.quit_button = tk.Button(self.data_mode_line_7_frame, text="QUIT", command=lambda:self.client.handle_msg('QUIT'))
        self.quit_button.pack(side=tk.LEFT)

        self.data_mode_line_8_frame = tk.Frame(self.data_mode_frame)
        self.data_mode_line_8_frame.grid(row=7, column=0, pady=10)
        self.rnfr_button = tk.Button(self.data_mode_line_8_frame, text="RNFR", command=self.handle_rnfr_button)
        self.rnfr_button.pack(side=tk.LEFT)
        self.rnto_button = tk.Button(self.data_mode_line_8_frame, text="RNTO", command=self.handle_rnto_button)
        self.rnto_button.pack(side=tk.LEFT)



        self.path_table_frame = tk.Frame(self.root)
        self.path_table_frame.pack(fill=tk.BOTH, expand=True)
        self.path_frame = tk.Frame(self.path_table_frame)
        self.path_frame.pack(side=tk.TOP, fill=tk.X)

        self.local_path_label = tk.Label(self.path_frame, text="Local Path")
        self.local_path_label.pack(side=tk.LEFT)
        self.local_path_entry = tk.Entry(self.path_frame)
        self.local_path_entry.pack(side=tk.LEFT, fill=tk.X, expand=True)
        # self.local_path_button = tk.Button(self.path_frame, text="Browse", command=lambda:self.on_local_path_button_click())
        # self.local_path_button.pack(side=tk.LEFT)

        self.server_path_label = tk.Label(self.path_frame, text="Server Path")
        self.server_path_label.pack(side=tk.LEFT)
        self.server_path_entry = tk.Entry(self.path_frame)
        self.server_path_entry.pack(side=tk.LEFT, fill=tk.X, expand=True)


        self.table_frame = tk.Frame(self.path_table_frame)
        self.table_frame.pack(fill=tk.BOTH, expand=True)
        self.local_table = ttk.Treeview(self.table_frame)
        self.local_table.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.separator = ttk.Separator(self.table_frame, orient=tk.VERTICAL)
        self.separator.pack(side=tk.LEFT, fill=tk.Y)

        self.server_table = ttk.Treeview(self.table_frame)
        self.server_table.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        self.local_table["columns"] = (0, 1, 2)

        # 设置列的标题
        self.local_table.heading(0, text="Name")
        self.local_table.heading(1, text="Size")
        self.local_table.heading(2, text="Modified")

        
        self.server_table['columns'] = ('Name', 'Size', 'Modified')
        self.server_table.heading("Name", text="Name")
        self.server_table.heading("Size", text="Size")
        self.server_table.heading("Modified", text="Modified")


        self.client = Client(self)
    def on_local_path_button_click(self):
        selected_directory = filedialog.askdirectory()
        # print("Selected directory:", selected_directory)

        # Insert the selected directory into local_path_entry
        self.local_path_entry.delete(0, tk.END)
        self.local_path_entry.insert(tk.END, selected_directory)
        # todo: update local table
        self.updata_local_table()
        # self.local_table.delete(*self.local_table.get_children())
        # for file in os.listdir(selected_directory):
        #     self.local_table.insert('', 'end', text=file)
        # self.local_table.insert('', 'end', text='test.txt')


    def updata_local_table(self):
        print('update local table')
        command = f"ls -l {self.local_path_entry.get()}"
        result = os.popen(command).read()
        self.local_table.delete(*self.local_table.get_children())
        # print(result)
        for line in result.split('\n'):
            if 'total' in line:
                continue
            if len(line) > 0:
                # print(line)
                info = line.split()
                name = info[-1]
                size = info[4]
                modified = ' '.join(info[5:8])
                self.local_table.insert('', 'end', values=(name, size, modified))

    def updata_server_table(self):
        print('update server table')
        self.server_table.delete(*self.server_table.get_children())
        self.client.server_socket.send(('PASV\r\n').encode())
        try:
            self.client.data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            if self.client.data_socket.fileno() < 0:
                print("Error creating socket")
            # self.client.server_socket.send(('PASV\r\n').encode())
            response = self.client.server_socket.recv(8192).decode('utf8')
            # print(f'FROM SERVER: {response}')
            # self.append_message_response_text(f'{response}')
            self.data_mode_status_text.delete(1.0, tk.END)
            self.data_mode_status_text.insert(tk.END, "PASV")

            if "227" in response:
                try:
                    pattern = re.compile(r'(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)')
                    h1 = pattern.search(response).group(1)
                    h2 = pattern.search(response).group(2)
                    h3 = pattern.search(response).group(3)
                    h4 = pattern.search(response).group(4)
                    p1 = pattern.search(response).group(5)
                    p2 = pattern.search(response).group(6)
                    ip = '.'.join([h1, h2, h3, h4])
                    port = int(p1) * 256 + int(p2)
                    # print(f'ip:{ip}, port:{port}')
                    server_addr = (ip, port)
                    self.client.server_addr = server_addr
                    self.client.data_socket.connect(self.client.server_addr)
                    # print(self.client.data_socket)
                    self.client.server_socket.send(('LIST\r\n').encode('utf8'))
                    # print("start listing")
                    response = self.client.server_socket.recv(8192).decode('utf8')
                    # print(f'response: {response}')
                    result = ''
                    if response.split(' ')[0] == '150':
                        # receive file
                        # print('receive')
                        while True:
                            data = self.client.data_socket.recv(8192).decode('utf8')
                            # print(data)
                            result += data
                            if not data:
                                break
                        if not "226" in response:
                            response = self.client.server_socket.recv(8192).decode('utf8')
                    # result = self.client.data_socket.recv(8192).decode('utf8')
                    for line in result.split('\n'):
                        if 'total' in line:
                            continue
                        if len(line) > 0:
                            # print(line)
                            info = line.split()
                            name = info[-1]
                            size = info[4]
                            modified = ' '.join(info[5:8])
                            self.server_table.insert('', 'end', values=(name, size, modified))
                except Exception as e:
                    # print("Error: invalid response:", e)
                    t = 1
        except Exception as e:
            # print("Error")
            t = 1

        

        self.client.data_socket = None
        

    def connect(self):
        server_ip = self.server_ip_entry.get()
        server_port = int(self.server_port_entry.get())
        username = self.username_entry.get().strip()
        password = self.password_entry.get().strip()
        self.client.connect_gui(server_ip, server_port)
        self.client.handle_rest(f'USER {username}')
        self.client.handle_rest(f'PASS {password}')
        # self.client.connect_gui('127.0.0.1', 21)
        # self.client.handle_rest('USER anonymous')
        # self.client.handle_rest('PASS 1')
        self.updata_local_table()
        self.updata_server_table()
        self.data_mode_status_text.delete(1.0, tk.END)
        self.data_mode_status_text.insert(tk.END, "None")


        # get self.root
        self.client.server_socket.send(("PWD"+'\r\n').encode('utf8'))
        response = self.client.server_socket.recv(8192).decode('utf8')
        # print(f'FROM SERVER: {response}')
        pattern = re.compile(r'[cC]urrent directory: (.*)')
        pattern_2 = re.compile(r'"(.*)" is current directory')
        # get current directory
        match = pattern.search(response)
        if match:
            self.server_path_entry.delete(0, tk.END)
            self.server_path_entry.insert(tk.END, pattern.search(response).group(1))
            self.client.root = pattern.search(response).group(1)
            # self.updata_server_table()
            # print(f'current directory: {pattern.search(response).group(1)}')
        
        match = pattern_2.search(response)
        if match:
            self.server_path_entry.delete(0, tk.END)
            self.server_path_entry.insert(tk.END, pattern_2.search(response).group(1))
            self.client.root = pattern_2.search(response).group(1)
            # self.updata_server_table()
        self.server_path_entry.delete(0, tk.END)
        self.server_path_entry.insert(tk.END, self.client.root)
        self.local_path_entry.delete(0, tk.END)
        self.local_path_entry.insert(tk.END, os.getcwd())


    def handle_port_button(self):
        command = simpledialog.askstring("PORT", "Enter PORT command")
        if command is not None:
            self.client.handle_msg(command)

    def handle_mkd_button(self):
        command = simpledialog.askstring("MKD", "Enter MKD command")
        if command:
            self.client.handle_msg(command)
    def handle_rmd_button(self):
        command = simpledialog.askstring("RMD", "Enter RMD command")
        if command:
            self.client.handle_msg(command)
    
    def handle_cwd_button(self):
        command = simpledialog.askstring("CWD", "Enter CWD command")
        if command:
            self.client.handle_msg(command)

    def handle_retr_button(self):
        command = simpledialog.askstring("RETR", "Enter RETR command")
        if command:
            self.client.handle_msg(command)

    def handle_stor_button(self):
        command = simpledialog.askstring("STOR", "Enter STOR command")
        if command:
            self.client.handle_msg(command)

    def handle_list_button(self):
        command = simpledialog.askstring("LIST", "Enter LIST command")
        if command:
            self.client.handle_msg(command)


    def handle_rnfr_button(self):
        command = simpledialog.askstring("RNFR", "Enter RNFR command")
        if command:
            self.client.handle_msg(command)

    def handle_rnto_button(self):
        command = simpledialog.askstring("RNT0", "Enter RNT0 command")
        if command:
            self.client.handle_msg(command)

    def append_message_response_text(self, message):
        self.message_response_text.insert(tk.END, message)
        self.message_response_text.see(tk.END)
        


class Client:
    def __init__(self, gui):
        self.server_socket = None
        self.data_socket = None
        self.data_mode = None
        self.server_addr = None
        self.gui = gui
        if self.gui:
            self.gui.data_mode_status_text.insert(tk.END, "None")

    def handle_rest(self, message):
        # print(f"handle_rest: {message}")
        if message == "":
            return
        self.server_socket.send((message+'\r\n').encode())
        file = self.server_socket.makefile()
        response = file.readline()
        if len(response) > 3 and response[3] == '-':
            while True:
                line = file.readline()
                response += line
                if line[3] != '-':
                    break
        
        print(f'FROM SERVER: {response}')
        if self.gui:
            self.gui.append_message_response_text(f'{response}')



    # handle PORT command
    def handle_port(self, message):
        # get ip and port from message
        # message: PORT h1,h2,h3,h4,p1,p2
        parameter = message.split(' ')[1]
        ip_port = parameter.split(',')
        ip = '.'.join(ip_port[:4])
        port = int(ip_port[5]) + int(ip_port[4]) * 256
        # print(type(ip))
        # print(f'ip:{ip}, port:{port}')

        # create a new socket
        self.data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if self.data_socket.fileno() < 0:
            print("Error creating socket")

        # set server address
        server_addr = (ip, port)
        self.data_socket.bind(server_addr)
        self.data_socket.listen()

        self.server_socket.send((message+'\r\n').encode('utf8'))
        response = self.server_socket.recv(8192).decode('utf8')
        print(f'FROM SERVER: {response}')
        self.data_mode = 'PORT'
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
            self.gui.data_mode_status_text.delete(1.0, tk.END)
            self.gui.data_mode_status_text.insert(tk.END, "PORT")



    # handle PASV command
    def handle_pasv(self, message):
        self.server_socket.send((message+'\r\n').encode())
        try:
            self.data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            if self.data_socket.fileno() < 0:
                print("Error creating socket")
            response = self.server_socket.recv(8192).decode('utf8')
            print(f'FROM SERVER: {response}')
            if self.gui:
                self.gui.append_message_response_text(f'{response}')
                self.gui.data_mode_status_text.delete(1.0, tk.END)
                self.gui.data_mode_status_text.insert(tk.END, "PASV")

            if "227" in response:
                try:
                    pattern = re.compile(r'(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)')
                    h1 = pattern.search(response).group(1)
                    h2 = pattern.search(response).group(2)
                    h3 = pattern.search(response).group(3)
                    h4 = pattern.search(response).group(4)
                    p1 = pattern.search(response).group(5)
                    p2 = pattern.search(response).group(6)
                    ip = '.'.join([h1, h2, h3, h4])
                    port = int(p1) * 256 + int(p2)
                    # print(f'ip:{ip}, port:{port}')
                    server_addr = (ip, port)
                    self.server_addr = server_addr
                    # self.data_socket.connect(server_addr)
                    self.data_mode = 'PASV'
                except Exception as e:
                    print("Error: invalid response:", e)
        except Exception as e:
            print("Error")

    # handle retr_thread
    def handle_retr_thread(self, message):
        if self.data_mode == "PORT":
            self.server_socket.send((message+'\r\n').encode('utf8'))
            self.data_socket, addr = self.data_socket.accept()
        elif self.data_mode == "PASV":
            self.server_socket.send((message+'\r\n').encode('utf8'))
            self.data_socket.connect(self.server_addr)
        else:
            self.server_socket.send((message+'\r\n').encode('utf8'))
        response = self.server_socket.recv(8192).decode('utf8')
        print(f'FROM SERVER: {response}')
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
        if response.split(' ')[0] == '150':
            # receive file
            filename = message.split(' ')[1].split('/')[-1]
            if self.gui:
                filename = os.path.join(self.gui.local_path_entry.get(), filename)
            f = open(filename, 'wb')
            while True:
                data = self.data_socket.recv(8192)
                if not data:
                    break
                f.write(data)
            f.close()
            self.data_socket.close()
            self.data_socket = None
            self.data_mode = None
            if self.gui:
                self.gui.data_mode_status_text.delete(1.0, tk.END)
                self.gui.data_mode_status_text.insert(tk.END, "None")
            # print("File received")
            if "226" in response:
                if self.gui:
                    self.gui.updata_local_table()
                    self.gui.updata_server_table()
                # print("File received")
                return
            response = self.server_socket.recv(8192).decode('utf8')
            if not response:
                return
            print(f'FROM SERVER: {response}')
            # print("time to return")
            if self.gui:
                self.gui.append_message_response_text(f'{response}')
                self.gui.updata_local_table()
                self.gui.updata_server_table()
            return

        # else:
        #     print("Error: no file to receive")
        if self.data_socket:
            self.data_socket.close()
            self.data_socket = None
            self.data_mode = None
        if self.gui:
            self.gui.data_mode_status_text.delete(1.0, tk.END)
            self.gui.data_mode_status_text.insert(tk.END, "None")

    # handle retr from the client
    def handle_retr(self, message):
        if self.gui:
                self.gui.data_mode_status_text.delete(1.0, tk.END)
                self.gui.data_mode_status_text.insert(tk.END, "None")
        process = Thread(target=self.handle_retr_thread, args=(message,))
        process.start()
        # process.join()
        # print("finish")
        
    # handle stor_thread
    def handle_stor_thread(self, message):
        filename = message.split(' ')[1]
        if self.gui:
            if len(filename.split('/')) <= 1:
                filename = os.path.join(self.gui.local_path_entry.get(), filename)
        if not os.path.isfile(filename):
            print("Error: file not found\n")
            if self.gui:
                self.gui.append_message_response_text(f"Error: file not found\n")
            return
        
        
        # first check if file exists
        # print("send")
        if self.data_mode == "PORT":
            self.server_socket.send((message+'\r\n').encode('utf8'))
            self.data_socket, addr = self.data_socket.accept()
        elif self.data_mode == "PASV":
            self.server_socket.send((message+'\r\n').encode('utf8'))
            self.data_socket.connect(self.server_addr)
        else:
            self.server_socket.send((message+'\r\n').encode('utf8'))
        response = self.server_socket.recv(8192).decode('utf8')
        print(f'FROM SERVER: {response}')
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
        

        if response.split(' ')[0] == '150':
            # send file
            # print("start sending")
            filename = message.split(' ')[1]
            f = open(filename, 'rb')
            while True:
                data = f.read(8192)
                if not data:
                    break
                self.data_socket.send(data)
            f.close()
            self.data_socket.close()
            self.data_socket = None
            self.data_mode = None
            if self.gui:
                self.gui.data_mode_status_text.delete(1.0, tk.END)
                self.gui.data_mode_status_text.insert(tk.END, "None")
            # print(f"File sent: {response}")
            if "226" in response:
                self.gui.updata_local_table()
                self.gui.updata_server_table()
                return
            response = self.server_socket.recv(8192).decode('utf8')
            if not response:
                return
            print(f'FROM SERVER: {response}')
            if self.gui:
                self.gui.append_message_response_text(f'{response}')
                self.gui.updata_local_table()
                self.gui.updata_server_table()
        else:
            t = 1
        
    # handle stor from the client
    def handle_stor(self, message):
        if self.gui:
                self.gui.data_mode_status_text.delete(1.0, tk.END)
                self.gui.data_mode_status_text.insert(tk.END, "None")
        process = Thread(target=self.handle_stor_thread, args=(message,))
        process.start()

    # handle list from the client
    def handle_list(self, message):
        if self.data_mode == "PORT":
            self.server_socket.send((message+'\r\n').encode('utf8'))
            self.data_socket, addr = self.data_socket.accept()
        elif self.data_mode == "PASV":
            self.server_socket.send((message+'\r\n').encode('utf8'))
            self.data_socket.connect(self.server_addr)
        else:
            self.server_socket.send((message+'\r\n').encode('utf8'))
        response = self.server_socket.recv(8192).decode('utf8')
        print(f'FROM SERVER: {response}')
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
        if response.split(' ')[0] == '150':
            # receive file
            while True:
                data = self.data_socket.recv(8192)
                if not data:
                    break
                print(data.decode('utf8'), end='')
                if self.gui:
                    self.gui.append_message_response_text(f'{data.decode("utf8")}')
                    self.gui.data_mode_status_text.delete(1.0, tk.END)
                    self.gui.data_mode_status_text.insert(tk.END, "None")
            self.data_socket.close()
            self.data_socket = None
            self.data_mode = None
            
            # print("List received")
            if "226" in response:
                return
            response = self.server_socket.recv(8192).decode('utf8')
            if not response:
                return
            print(f'FROM SERVER: {response}')
            if self.gui:
                self.gui.append_message_response_text(f'{response}')
        # else:
        #     print("Error: no list to receive")

    # handle mkd from the client
    def handle_mkd(self, message):
        self.server_socket.send((message+'\r\n').encode('utf8'))
        response = self.server_socket.recv(8192).decode('utf8')
        print(f'FROM SERVER: {response}')
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
            if message[4] != '/':
            # todo: update server table
                self.gui.updata_server_table()
            # self.gui.server_table.insert('', 'end', text=message[4:])

    def handle_rmd(self, message):
        self.server_socket.send((message+'\r\n').encode('utf8'))
        response = self.server_socket.recv(8192).decode('utf8')
        print(f'FROM SERVER: {response}')
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
            if message[4] != '/':
                self.gui.updata_server_table()
            # todo: update server table

    def handle_rnto(self, message):
        self.server_socket.send((message+'\r\n').encode('utf8'))
        response = self.server_socket.recv(8192).decode('utf8')
        print(f'FROM SERVER: {response}')
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
            self.gui.updata_server_table()

    def handle_cwd(self, message):
        # print("cwd")
        self.server_socket.send((message+'\r\n').encode('utf8'))
        response = self.server_socket.recv(8192).decode('utf8')
        print(f'FROM SERVER: {response}')
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
        pattern = re.compile(r'[cC]urrent directory: (.*)')
        pattern_2 = re.compile(r'"(.*)" is current directory')
        # get current directory
        match = pattern.search(response)
        if match:
            if self.gui:
                self.gui.server_path_entry.delete(0, tk.END)
                self.gui.server_path_entry.insert(tk.END, pattern.search(response).group(1))
                self.gui.updata_server_table()
            print(f'current directory: {pattern.search(response).group(1)}')
        
        match = pattern_2.search(response)
        if match:
            if self.gui:
                self.gui.server_path_entry.delete(0, tk.END)
                self.gui.server_path_entry.insert(tk.END, pattern_2.search(response).group(1))
                self.gui.updata_server_table()

    # handle msg from the client
    def handle_msg(self, message):
        cmd = message.split(' ')[0]
        if cmd == 'PORT':
            self.handle_port(message)
        elif cmd == 'RETR':
            self.handle_retr(message)
        elif cmd == 'PASV':
            self.handle_pasv(message)
        elif cmd == 'STOR':
            self.handle_stor(message)
        elif cmd == 'LIST':
            self.handle_list(message)
        elif cmd == 'MKD':
            self.handle_mkd(message)
        elif cmd == 'RMD':
            self.handle_rmd(message)
        elif cmd == 'CWD':
            self.handle_cwd(message)
        elif cmd == 'RNTO':
            self.handle_rnto(message)
        else:
            self.handle_rest(message)


    # def handle_pasv(self):
    def connect(self, server_ip, server_port, root='/tmp'):
        self.root = root
        # create socket
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if self.server_socket.fileno() < 0:
            print("Error creating socket")
        # set server address
        server_addr = (server_ip, server_port)

        # connect to server
        try:
            self.server_socket.connect(server_addr)
        except socket.error as e:
            # print(e)
            exit(1)
        # send message to server
        response = self.server_socket.recv(8192).decode()
        if not response:
            print("Connection closed by server")
        print("FROM SERVER: ", response)
        if self.gui:
            self.gui.append_message_response_text(f'{response}')

        while True:
            message = input('ftp>')
            # remove \n at the end and add \r\n
            message = message.rstrip()
            # message += '\r\n'
            # remove \n at the end and add \0
            # message = message.rstrip()
            # print(f'message after rstrip: {message}')
            self.handle_msg(message)
            if message == 'QUIT':
                break
            if message == 'ABOR':
                break
        # print('no response')
        # close socket
        self.server_socket.close()

    # connect gui
    def connect_gui(self, server_ip, server_port, root='/tmp'):
        self.root = root
        # create socket
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if self.server_socket.fileno() < 0:
            print("Error creating socket")
        # set server address
        server_addr = (server_ip, server_port)

        # connect to server
        try:
            self.server_socket.connect(server_addr)
        except socket.error as e:
            print(e)
            exit(1)
        # send message to server
        response = self.server_socket.recv(8192).decode()
        if not response:
            print("Connection closed by server")
        print("FROM SERVER: ", response)
        if self.gui:
            self.gui.append_message_response_text(f'{response}')
        
        
        # local_files = os.listdir(os.getcwd())
        # server_files = self.get_server_files(self.root)
        # todo：显示文件



#  parse command line arguments
def parse_args():
    parser = argparse.ArgumentParser(description="FTP Client")
    parser.add_argument('--no_gui', action="store_true", help="Disable GUI mode")
    parser.add_argument('--port', type=int, default=21)
    parser.add_argument('--root', type=str, default='/tmp')

    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()
    if args.no_gui:
        port = args.port
        root = args.root
        client = Client(gui=None)
        client.connect('166.111.83.113', port, root)
        # client.connect('127.0.0.1', port, root)

    # client = Client()
    # client.connect('127.0.0.1', 21)
    # client.connect('166.111.83.113', 21)
    else:
        root = tk.Tk()
        client_gui = ClientGUI(root)
        root.mainloop()
    

