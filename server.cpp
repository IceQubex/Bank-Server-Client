#include<iostream>
#include<fstream>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<pthread.h>

using namespace std;

//create the Transaction structure
struct Transaction{
	int timestamp, acc_no, amount;
        char action;
};

//create the Record structure
struct Record{
	int acc_no, balance;
	char name[10];
};

//integer to keep track of the number of accounts
int number_of_acc = 0;

//create the global records array that the information from "Records.txt" will be stored in
Record records[100];

//create the mutex 
pthread_mutex_t lock;

//declare and define the thread function
void *thread_func(void *arg)
{
	cout<<"\n\nConnection succcessfully established with client.\n"<<endl;
	
	//declare a pointer to the Transaction object and use it to handle the arguments passed in the thread
	int client_socket = *(int *) arg;
	Transaction current;
	char status[21];

	//repeatedly receive transactions from client in a while loop
	while(1)
	{
		//stores each iteration of a receive call into the current Transaction object
		recv(client_socket, &current, sizeof(current), 0);
		int i;

		//If client sends timestamp=0, it indicates all information from client has been sent, and thread can be destroyed
		if (current.timestamp == -1)
			break;
		
		//Run through each bank account to find matching account to make transaction on
		for(i=0;i<number_of_acc;i++)
		{
			if (records[i].acc_no == current.acc_no)
			{
				cout<<"\n\nCurrently operating in "<<records[i].name<<"'s account: "<<endl;
				if (current.action == 'w')
				{
					if (records[i].balance>=current.amount)
					{
						cout<<"Withdrawing "<<current.amount<<" dollars from "<<records[i].name<<"'s account."<<endl;
						cout<<"The old balance is: "<<records[i].balance<<endl;
						pthread_mutex_lock(&lock);
						records[i].balance-=current.amount;
						pthread_mutex_unlock(&lock);
						cout<<"The new balance is: "<<records[i].balance<<endl<<endl;
						strcpy(status, "Transaction success!");
					}
					else
					{
						cout<<"Unable to withdraw "<<current.amount<<" dollars from "<<records[i].name<<"'s account due to insufficient balance."<<endl;
					cout<<"The balance is: "<<records[i].balance<<endl<<endl;
						strcpy(status, "Transaction failure!");
					}
				}		
				else
				{
					cout<<"Depositing "<<current.amount<<" dollars in "<<records[i].name<<"'s account."<<endl;
					cout<<"The old balance is: "<<records[i].balance<<endl;
					pthread_mutex_lock(&lock);
					records[i].balance+=current.amount;
					pthread_mutex_unlock(&lock);
					cout<<"The new balance is: "<<records[i].balance<<endl<<endl;
					strcpy(status, "Transaction success!");
				}

				//send the transaction status and the updated records back to the client
				send(client_socket, &records[i], sizeof(records[i]), 0);
				send(client_socket, &status, sizeof(status), 0);
			}
		}
	}
	
	//exit the thread without any return
	pthread_exit(NULL);
}

//thread to display idle status 
void *idle_func(void *arg)
{
	while(1)
	{
		cout<<endl<<endl<<"Waiting for connections"<<flush;
		int i;
		for(i=0;i<=4;i++)
		{
			sleep(1);
			cout<<"."<<flush;
		}
		sleep(15);
	}
}

//thread to add interest to all the bank accounts after fixed interval of time
void *interest_func(void *arg)
{
	while(1)
	{
		sleep(30);
		cout<<endl<<endl<<"Bank server now calculating and adding interest to accounts!"<<endl<<endl;
		int i;
		for(i=0;i<number_of_acc;i++)
		{
			pthread_mutex_lock(&lock);
			records[i].balance*=1.1;
			pthread_mutex_unlock(&lock);
			cout<<"Name: "<<records[i].name<<endl<<"Account Number: "<<records[i].acc_no<<endl<<"New Balance: "<<records[i].balance<<endl<<endl;
		}
		cout<<endl<<endl<<"Bank server is done with adding interest!"<<endl<<endl;
	}
}

//main process
int main(int argc, char *argv[])
{
	//check for proper calling for the server
	if (argc<2)
	{
		cout<<"Usage: "<<argv[0]<<" <port number>"<<endl;
		exit(1);
	}
	
	//accept input from the "Records.txt" file and store it in an array of Record structure
	ifstream rec_file("Records.txt");
	while(rec_file>>records[number_of_acc].acc_no>>records[number_of_acc].name>>records[number_of_acc].balance)
		number_of_acc++;
	rec_file.close();
	
	//display the original account information
	int i;
	for (i=0;i<=number_of_acc-1;i++)
		cout<<"\n\nName: "<<records[i].name<<"\nAccount Number: "<<records[i].acc_no<<"\nCurrent Balance: "<<records[i].balance<<endl;
	
	//create the server socket
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	//assign the socket parameters
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[1]));
	server_address.sin_addr.s_addr = INADDR_ANY;

	//connect to the socket and start listening
	bind(server_socket,(struct sockaddr *) &server_address, sizeof(server_address));
	listen(server_socket, 5);
	
	//accept the incoming connection from client process
	int client_socket;
	pthread_t thread_id;
	
	//for waiting animation
	pthread_t wait_id;
	pthread_create(&wait_id, NULL, idle_func, NULL);
	pthread_detach(wait_id);
	pthread_t interest_id;
	pthread_create(&interest_id, NULL, interest_func, NULL);
	pthread_detach(interest_id);

	//receive data from the connected client process
	while(true)
	{
		
		client_socket = accept(server_socket, NULL, NULL);
		if((pthread_create(&thread_id, NULL, thread_func, &client_socket))!=0)
				cout<<"Error in creating thread"<<endl;
		pthread_detach(thread_id);
	}
	
	//close and exit the server process
	close(server_socket);
	return 0;
}

