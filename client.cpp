#include<iostream>
#include<fstream>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>
#include<sys/time.h>

#define PORT 12345

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

//main function definition
int main(int argc, char *argv[])
{
	
	if(argc<2)
	{
		cout<<"Please provide a request rate in seconds."<<endl;
		exit(1);
	}
	
	//create instance of Transaction
	Transaction current;

	//create reply message and reply Record object
	char reply[50];
	Record received_rec;

	//read the parameter from the commandline which sets the request rate
	float req = atof(argv[1]);

	//create the timeval structures and other variables for time assessment
	timeval start, end;
	int micro_seconds[100];
	int i;
	int num_of_trans=0;
	long sum=0;
	float average;

	//create the client socket
        int client_socket;
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
		
	//assign the socket parameters
        struct sockaddr_in client_address;
	client_address.sin_family = AF_INET;
        client_address.sin_port = htons(PORT);
        client_address.sin_addr.s_addr=INADDR_ANY;

        //connect to the socket and send data
        connect(client_socket, (struct sockaddr *) &client_address, sizeof(client_address));
	
	//open the Transactions.txt file
	ifstream trans_file("Transactions.txt");

	//initialise while loop to read from the Transactions.txt, store in Transaction object & send the data to server
	while(trans_file>>current.timestamp>>current.acc_no>>current.action>>current.amount)
	{
		
		//read the transaction data and display the data
		cout<<endl<<"Timestamp: "<<current.timestamp<<endl<<"Account Number: "<<current.acc_no<<endl;
		if (current.action == 'w')
			cout<<"Transaction: Withdraw "<<current.amount<<" dollars."<<endl;
		else
			cout<<"Transaaction: Deposit "<<current.amount<<" dollars."<<endl;
		//initiate the timer counter
		gettimeofday(&start, NULL);

		//send the transaction data to the server
		send(client_socket, &current, sizeof(current), 0);	
		
		//wait for replies from the server regarding the transaction status and the updated account details
		recv(client_socket, &received_rec, sizeof(received_rec), 0);
		recv(client_socket, &reply, sizeof(reply), 0);

		//stop the timer counter
		gettimeofday(&end, NULL);

		//calculate time for each request
		micro_seconds[num_of_trans] = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);

		//display the replies from the server
		cout<<endl<<"Message from Server:"<<endl<<reply<<endl;
		cout<<"There is now "<<received_rec.balance<<" dollars in "<<received_rec.name<<"'s account. (Account Number: "<<received_rec.acc_no<<")"<<endl;
		cout<<"The time taken is "<<micro_seconds[num_of_trans]<<" microseconds."<<endl;
		
		//throttle the request rate according to the input parameters
		usleep(1000000*req);
		num_of_trans++;
	}

	//transaction object used to signal the end of sending data from client
	Transaction final;
	final.timestamp = -1; final.acc_no = 0; final.action = 'x'; final.amount = 0;
	send(client_socket, &final, sizeof(final), 0);

	//calculate the average time taken for each transaction
	for(i=0;i<num_of_trans;i++)
	{
		sum += micro_seconds[i];
	}
	average = sum/num_of_trans;
	cout<<endl<<endl<<"The average time taken per transaction is "<<average<<" microseconds."<<endl;

	//close the file
	trans_file.close();

	//close and exit the client process
	close(client_socket);
	return 0;
}

