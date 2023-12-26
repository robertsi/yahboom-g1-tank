#include <iostream>
#include <regex>
#include <g1-tank/tank.hpp>

void G1Tank::parseCommand(const std::string& command)
{
    printf("Received command: %s\n", command.c_str());
    const std::regex regEx("^\\$(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)#$");
    std::smatch group_match;
    if(std::regex_match(command, group_match, regEx))
    {
        printf("main command\n");
        printf("command valid\n");
        std::ssub_match movement_match = group_match[1];
        std::string movement = movement_match.str();
        printf("Movement %s\n", movement.c_str());

        if(movement == "0")
        {
            this->stop();
        } else if (movement == "1")
        {
            this->moveForward();
        }
        else if (movement == "2")
        {
            this->moveBack();
        }
        else if (movement == "3")
        {
            this->moveLeft();
        }
        else if (movement == "4")
        {
            this->moveRight();
        }

        std::ssub_match spin_match = group_match[2];
        std::string spin = spin_match.str();

        if(spin == "1")
        {
            this->spinLeft();
        } else if (spin == "2")
        {
            this->spinRight();
        }


        std::ssub_match buzzer_match = group_match[3];
        std::string buzzer = buzzer_match.str();
        if (buzzer == "1")
        {
            this->whistle();
        }

        std::ssub_match servo_match = group_match[5];
        std::string servo = servo_match.str();
        printf("Servo %s\n", servo.c_str());

        if(servo == "1")
        {
                this->setServoAngle(Servo::FRONT, 180);
        } else if(servo == "2")
        {
                this->setServoAngle(Servo::FRONT, 0);
        }
        else if(servo == "3") //up
        {
                this->setServoAngle(Servo::UP_DOWN, this->getServoAngle(Servo::UP_DOWN) + 2);
        }
        else if(servo == "4") //down
        {
                this->setServoAngle(Servo::UP_DOWN, this->getServoAngle(Servo::UP_DOWN) - 2);
        }
        else if(servo == "5")
        {
                this->setServoAngle(Servo::UP_DOWN, 90);
        }
        else if(servo == "6")
        {
                this->setServoAngle(Servo::LEFT_RIGHT, this->getServoAngle(Servo::LEFT_RIGHT) + 2);
        }
        else if(servo == "7")
        {
                this->setServoAngle(Servo::LEFT_RIGHT, this->getServoAngle(Servo::LEFT_RIGHT) - 2);
        }
        else if(servo == "8")
        {
                //this->setServoAngle(Servo::LEFT_RIGHT, 90);
        }

        std::ssub_match light_match = group_match[7];
        std::string light = light_match.str();

        if(light == "1")
        {
            this->color_led_pwm(255, 255, 255);
            this->setServoAngle(Servo::FRONT, 90);
            this->setServoAngle(Servo::UP_DOWN, 90);
            this->setServoAngle(Servo::LEFT_RIGHT, 90);
        } else if (light == "2")
        {
            this->color_led_pwm(255, 0, 0);
        }
        else if (light == "3")
        {
            this->color_led_pwm(0, 255, 0);
        }
        else if (light == "4")
        {
            this->color_led_pwm(0, 0, 255);
        }
        else if (light == "5")
        {
            this->color_led_pwm(255, 255, 0);
        }
        else if (light == "6")
        {
            this->color_led_pwm(0, 255, 255);
        }
        else if (light == "7")
        {
            this->color_led_pwm(255, 0, 255);
        }       
        else
        {
            this->color_led_pwm(0, 0, 0);
        }

        servo_match = group_match[9];
        servo = servo_match.str();
        if(servo == "1")
        {
                this->setServoAngle(Servo::FRONT, 90);
        }

        std::ssub_match speed_match = group_match[4];
        std::string speed = speed_match.str();

        if(speed == "1")
        {
            this->setSpeed(this->getSpeed() + 10);
        }
        else if (speed == "2")
        {
             this->setSpeed(this->getSpeed() - 10);
        }
        printf("Speed %d\n", this->getSpeed());

        
    }
    else if (command.rfind("CLR", 5) == 5)
    {
          printf("CLR command\n");
        const std::regex regEx("^\\$4WD,CLR(\\d+),CLG(\\d+),CLB(\\d+)#$");
        std::smatch group_match;
        if(std::regex_match(command, group_match, regEx))
        {
            std::ssub_match red_match = group_match[1];
            int red = stoi(red_match.str());

            std::ssub_match green_match = group_match[2];
            int green = stoi(green_match.str());

            std::ssub_match blue_match = group_match[3];
            int blue = stoi(blue_match.str());

            this->color_led_pwm(red, green, blue);
        }
    }
    else if (command.rfind("PTZ", 5) == 5)
    {
        printf("PTZ command\n");
        const std::regex regEx("^\\$4WD,PTZ(\\d+)#$");
        std::smatch group_match;
        if(std::regex_match(command, group_match, regEx))
        {
            std::ssub_match ptz_match = group_match[1];
            int angle = stoi(ptz_match.str());
           

            printf("Setting servo angle %d\n", angle);
            this->setServoAngle(Servo::FRONT, angle);
        }
    }
    else {
        printf("command invalid\n");
    }
    
}

void G1Tank::TCP_receive(thread_info& ti)
{
          // thread_info& ti = *(thread_info*)arg;
    printf("=============================================\n");
	printf("connect_fd : %d\n", ti.socket_descriptor);
	printf("client IP : %s\n", inet_ntoa(ti.client_addr.sin_addr));
	printf("client port : %d\n", ntohs(ti.client_addr.sin_port));

    setSocketBlockingEnabled(ti.socket_descriptor, false);
    while(!_stopThreads)
    {
        char recvbuf[1024];
        auto n = recv(ti.socket_descriptor, recvbuf, sizeof(recvbuf),0);
        // printf("%d\n", n);
        if (n == -1)
        {
            continue;
        }
        if(n < 0)
        {
            perror("Fail to recv!\n");	
            break;
            //	exit(EXIT_FAILURE);
        }else if(n == 0){
            printf("clinet_recv is not connect\n");	
            break;
            //	exit(EXIT_FAILURE);
        }		
        // printf("Recv %d bytes : %s\n", n, recvbuf);
        std::string command(recvbuf, n);
        parseCommand(command);

        // Data_Pack();
        // if (NewLineReceived == 1)
        // {S
        //     tcp_data_parse();
        // }
    }
    printf("Terminating thread\n");
    shutdown(ti.socket_descriptor, SHUT_RDWR);
    close(ti.socket_descriptor);
    pthread_exit(nullptr);
}