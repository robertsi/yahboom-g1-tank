// https://github.com/microsoft/vscode-cmake-tools/tree/main/docs#cmake-tools-for-visual-studio-code-documentation
// /usr/lib/aarch64-linux-gnu/liblgpio.so
// /usr/include/lgpio.h
// sudo find / -name Find*.cmake
// https://indico.jlab.org/event/420/contributions/7961/attachments/6507/8734/CMakeSandCroundtable.slides.pdf
// https://hsf-training.github.io/hsf-training-cmake-webpage/
// https://gitlab.com/CLIUtils/modern-cmake/tree/master/examples/extended-project
// https://www.classcentral.com/course/freecodecamp-c-programming-course-beginner-to-advanced-93327
// https://developers.slashdot.org/story/21/04/17/009241/linus-torvalds-says-rust-closer-for-linux-kernel-development-calls-ca-crap-language#comments

#include <iostream>
// #include <map>
// #include <lgpio.h>
#include <signal.h>
// #include <unistd.h> // usleep
#include <g1-tank/tank.hpp>
// #include <gpiozero/gpiozero.hpp>
// #include <sys/socket.h>

// #include <thread>
// #include <regex>

int sigTermReceived = false;

bool SetSocketBlockingEnabled(int fd, bool blocking)
{
   if (fd < 0) return false;

#ifdef _WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

void signalHandler( int signum ) {
   std::cout << "Interrupt signal (" << signum << ") received.\n";

   // cleanup and close up stuff here  
   // terminate program  

    sigTermReceived = true;
   //exit(signum);

}

int main(int, char**){

     // register signal SIGINT and signal handler  ne
    signal(SIGINT, signalHandler);  
    // std::ios::sync_with_stdio(false);
    // std::cout << "Hello, from yahboom-tank-pi!\n";

    //    std::map<int, char> M = { { 1, 'a' },
    //                           { 2, 'b' } };
 
    // // Check if M has key 2
    // if (M.contains(2)) {
    //     std::cout << "Found\n";
    // }
    // else {
    //     std::cout << "Not found\n";
    // }
    // return 0;
    //  gpiozero::sayHello();
    // gpiozero::Pin* pin = new gpiozero::PiPin();
    // std::cout << pin << std::endl;
    // delete pin;

    // auto pin2 = gpiozero::PiPin();
    // std::cout << pin2 << std::endl;

    // auto tank = G1Tank();
    // tank.sayHello();
    // servo();
    // while(true)
    // {
    //     printf("waiting\n");
    //     // colorRGB();
    // }
    G1Tank tank;
    auto serial_fd = lgSerialOpen("/dev/ttyAMA0", 9600, 0);
    if (serial_fd < 0)
    {
        // fprintf(stderr, "Uable to open serial device: %s\n", strerror(errno));
        perror("Failed to open serail");
        exit(EXIT_FAILURE);
    }


    char serialBuffer[1024];
    while(!sigTermReceived)
    {
        auto bytes_read = lgSerialRead(serial_fd, serialBuffer, sizeof(serialBuffer));
        if(bytes_read > 0)
        {
            // printf("bytes received  %d\n", bytes_read);
            std::string command(serialBuffer, bytes_read);
            // printf("receive command %s\n", command.c_str());
            tank.parseCommand(command);
        }
    }

    lgSerialClose(serial_fd);

    //1.Create a listening socket through a socket
	auto listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0)
	{
		perror("Fail to socket");	
		exit(EXIT_FAILURE);
	}

    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi("8888"));
    my_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    //3.Bind ip and port
	if(bind(listen_fd, ( struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
	{
		perror("Fail to bind");	
		exit(EXIT_FAILURE);
	}

    //4.Listen for client connections
	listen(listen_fd, 5);
	printf("Listen....\n");


    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
   
    SetSocketBlockingEnabled(listen_fd, false);
   
    while(!sigTermReceived)
	{
         
        //5.Ready to receive client connection requests
        auto connect_fd = accept(listen_fd,(struct sockaddr *)&client_addr, &client_addr_len);

        // printf("connect_fd %d\n", connect_fd);
        if(connect_fd == EWOULDBLOCK || connect_fd == EAGAIN || connect_fd == -1)
        {
            continue;
        }
        thread_info ti;
        ti.socket_descriptor = connect_fd;
        ti.client_addr = client_addr;
        if(connect_fd < 0)
		{
			perror("Fail to accept");	
			exit(EXIT_FAILURE);
		}

        // pthread_create(&ti.thread_id, nullptr, do_client_recv, (void *)&ti);

        // auto& thread = threads.emplace_back(do_client_recv, std::ref(ti));
        tank.startReceive(ti);
        // std::thread thread();
        // thread.detach();

        
    }
    shutdown(listen_fd, SHUT_RDWR);
    close(listen_fd);
}


