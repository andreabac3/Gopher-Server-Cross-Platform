/*
	TCP Echo server example in winsock
	Live Server on port 8888
*/
#include&lt;stdio.h&gt;
#include&lt;winsock2.h&gt;

#pragma comment(lib, &quot;ws2_32.lib&quot;) //Winsock Library

int main(int argc , char *argv[])
{
    WSADATA wsa;
    SOCKET master , new_socket , client_socket[30] , s;
    struct sockaddr_in server, address;
    int max_clients = 30 , activity, addrlen, i, valread;
    char *message = "ECHO Daemon v10";

    //size of our receive buffer, this is string length.
    int MAXRECV = 1024;
    //set of socket descriptors
    fd_set readfds;
    //1 extra for null character, string termination
    char *buffer;
    buffer =  (char*) malloc((MAXRECV + 1) * sizeof(char));

    for(i = 0 ; i &lt; 30;i++)
    {
        client_socket[i] = 0;
    }

    printf("\nInitialising Winsock...&quot");
    if (WSAStartup(MAKEWORD(2,2),&amp;wsa) != 0)
    {
        printf(&quot;Failed. Error Code : %d&quot;,WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    printf("Initialised.");

    //Create a socket
    if((master = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf(&quot;Could not create socket : %d&quot; , WSAGetLastError());
        exit(EXIT_FAILURE);
    }


    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(master ,(struct sockaddr *)&amp ;server , sizeof(server)) == SOCKET_ERROR)
    {
        printf(&quot;Bind failed with error code : %d&quot; , WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    puts(&quot;Bind done&quot;);

    //Listen to incoming connections
    listen(master , 3);

    //Accept and incoming connection
    puts(&quot;Waiting for incoming connections...&quot;);

    addrlen = sizeof(struct sockaddr_in);

    while(TRUE)
    {
        //clear the socket fd set
        FD_ZERO(&amp;readfds);

        //add master socket to fd set
        FD_SET(master, &amp;readfds);

        //add child sockets to fd set
        for (  i = 0 ; i &lt; max_clients ; i++)
        {
            s = client_socket[i];
            if(s &gt; 0)
            {
                FD_SET( s , &amp;readfds);
            }
        }

        //wait for an activity on any of the sockets, timeout is NULL , so wait indefinitely
        activity = select( 0 , &amp;readfds , NULL , NULL , NULL);

        if ( activity == SOCKET_ERROR )
        {
            printf(&quot;select call failed with error code : %d&quot; , WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master , &amp;readfds))
        {
            if ((new_socket = accept(master , (struct sockaddr *)&amp;address, (int *)&amp;addrlen))&lt;0)
            {
                perror(&quot;accept&quot;);
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf(&quot;New connection , socket fd is %d , ip is : %s , port : %d \n&quot; , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )
            {
                perror(&quot;send failed&quot;);
            }

            puts(&quot;Welcome message sent successfully&quot;);

            //add new socket to array of sockets
            for (i = 0; i &lt; max_clients; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf(&quot;Adding to list of sockets at index %d \n&quot; , i);
                    break;
                }
            }
        }

        //else its some IO operation on some other socket :)
        for (i = 0; i &lt; max_clients; i++)
        {
            s = client_socket[i];
            //if client presend in read sockets
            if (FD_ISSET( s , &amp;readfds))
            {
                //get details of the client
                getpeername(s , (struct sockaddr*)&amp;address , (int*)&amp;addrlen);

                //Check if it was for closing , and also read the incoming message
                //recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).
                valread = recv( s , buffer, MAXRECV, 0);

                if( valread == SOCKET_ERROR)
                {
                    int error_code = WSAGetLastError();
                    if(error_code == WSAECONNRESET)
                    {
                        //Somebody disconnected , get his details and print
                        printf(&quot;Host disconnected unexpectedly , ip %s , port %d \n&quot; , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                        //Close the socket and mark as 0 in list for reuse
                        closesocket( s );
                        client_socket[i] = 0;
                    }
                    else
                    {
                        printf(&quot;recv failed with error code : %d&quot; , error_code);
                    }
                }
                if ( valread == 0)
                {
                    //Somebody disconnected , get his details and print
                    printf(&quot;Host disconnected , ip %s , port %d \n&quot; , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    closesocket( s );
                    client_socket[i] = 0;
                }

                    //Echo back the message that came in
                else
                {
                    //add null character, if you want to use with printf/puts or other string handling functions
                    buffer[valread] = '&#92;&#48;';
                    printf(&quot;%s:%d - %s \n&quot; , inet_ntoa(address.sin_addr) , ntohs(address.sin_port), buffer);
                    send( s , buffer , valread , 0 );
                }
            }
        }
    }

    closesocket(s);
    WSACleanup();

    return 0;
}