#include<iostream>
#include<fstream>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

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
	//error handling for incorrect number of arguments
	if(argc<2)
	{
		cout<<"Usage: "<<argv[0]<<" <port number>"<<endl;
		exit(1);
	}
	
	//create instance of Transaction
	Transaction current;

	//create reply message and reply Record object
	char reply[50];
	Record received_rec;

	//create the client socket
        int client_socket;
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
		
	//assign the socket parameters
        struct sockaddr_in client_address;
	client_address.sin_family = AF_INET;
        client_address.sin_port = htons(atoi(argv[1]));
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
		
		//send the transaction data to the server
		send(client_socket, &current, sizeof(current), 0);	
		
		//wait for replies from the server regarding the transaction status and the updated account details
		recv(client_socket, &received_rec, sizeof(received_rec), 0);
		recv(client_socket, &reply, sizeof(reply), 0);

		//display the replies from the server
		cout<<endl<<"Message from Server:"<<endl<<reply<<endl;
		cout<<"There is now "<<received_rec.balance<<" dollars in "<<received_rec.name<<"'s account. (Account Number: "<<received_rec.acc_no<<")"<<endl;
	}

	//transaction object used to signal the end of sending data from client
	Transaction end;
	end.timestamp = -1; end.acc_no = 0; end.action = 'x'; end.amount = 0;
	send(client_socket, &end, sizeof(end), 0);

	//close the file
	trans_file.close();

	//close and exit the client process
	close(client_socket);
	return 0;
}

