#pragma once

#include <iostream>
#include <memory>
#include <lgpio.h>
#include <vector>
#include <unistd.h> //usleep

#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>



 struct thread_info {
           pthread_t thread_id;
           sockaddr_in client_addr;
           int socket_descriptor;
 };

class G1Tank {
public:
    struct Options {
        Options() : 
        front_servo_pin(23),
        up_down_servo_pin(9),
        lef_right_servo_pin(11) {
        };

        int front_servo_pin;
        int up_down_servo_pin;
        int lef_right_servo_pin;
        
        int left_motor_go = 20;
        int left_motor_back = 21;
        int left_motor_pwm = 16;

        int right_motor_go = 19;
        int right_motor_back = 26;
        int right_motor_pwm = 13;

        int buzzer_pin = 8;


        int led_red_pin = 22;
        int led_green_pin = 27;
        int led_blue_pin = 24;
    };

    class Pin;
   
    class Chip
    {
        public:
        Chip(int gpioDev) : _hChip(lgGpiochipOpen(4) )
        {
            // _hChip = lgGpiochipOpen(4);
            if( _hChip < LG_OKAY)
            {
                throw std::invalid_argument("Failed to open GPIO chip!");
            }
            // _pins.reserve(5);
            // throw std::invalid_argument("fake except!");
        }

        G1Tank::Pin& ClaimOutput(int LFLAGS, int gpio, int level)
        {
             int res = lgGpioClaimOutput(_hChip, LFLAGS, gpio, level);
             if(res != LG_OKAY)
             {
                  throw std::invalid_argument("failed to claim output pin!");
             }       
           
            auto& pin = _pins.emplace_back(*this, gpio);
            return pin;

        }

        int getHandle() {
            return _hChip;
        }

        ~Chip()
        {
            printf("destructing chip\n");
            //ensure that pins are destroyed before chip
            _pins.clear();
            if( _hChip >= LG_OKAY)
            {
                printf("closing chip\n");
                lgGpiochipClose(_hChip);
            }
        }
        private:
        int _hChip;
        std::vector<G1Tank::Pin> _pins{};
    };

    class Pin 
    {
        public:
        Pin(G1Tank::Chip& chip, int gpio) :  _chip(chip), _pin(gpio)
        {
           
    
        }

        ~Pin()
        {
            
            if(_pin >= 0)
            {
                printf("Freeing pin %d\n", _pin);
                if(lgGpioFree(_chip.getHandle() ,_pin) == LG_OKAY)
                {
                    printf("Freed pin %d\n", _pin);
                }
                else
                {
                    printf("Failed to free pin %d\n", _pin);
                }
            }
        }

        Pin(const Pin&) = delete; // disable copy constructor
        Pin(Pin&& other) : _chip(other._chip), _pin(other._pin) {
            other._pin = -1;
            printf("pin %d being moved\n", _pin);
        }
        Pin& operator=(const Pin& other) = default;

        private:
        int _pin;
        G1Tank::Chip& _chip;
    };

    G1Tank(const G1Tank::Options &options = G1Tank::Options()) : _chip(4), _options( options) {
        printf("constructing G1Tank\n");

        int lFlags = 0;
        _chip.ClaimOutput(lFlags, _options.front_servo_pin, 0);
        _chip.ClaimOutput(lFlags, _options.up_down_servo_pin, 0);
        _chip.ClaimOutput(lFlags, _options.lef_right_servo_pin, 0);

        _chip.ClaimOutput(lFlags, _options.left_motor_go, 0);
        _chip.ClaimOutput(lFlags, _options.left_motor_back, 0);
        _chip.ClaimOutput(lFlags, _options.left_motor_pwm, 1);

        _chip.ClaimOutput(lFlags, _options.right_motor_go, 0);
        _chip.ClaimOutput(lFlags, _options.right_motor_back, 0);
        _chip.ClaimOutput(lFlags, _options.right_motor_pwm, 1);

        _chip.ClaimOutput(lFlags, _options.buzzer_pin, 1);

        _chip.ClaimOutput(lFlags, _options.led_red_pin, 0);
        _chip.ClaimOutput(lFlags, _options.led_green_pin, 0);
        _chip.ClaimOutput(lFlags, _options.led_blue_pin, 0);

        setSpeed(_speed);
        setServoAngle(Servo::FRONT, _frontServoAngle);
        setServoAngle(Servo::UP_DOWN, _upDownServoAngle);
        setServoAngle(Servo::LEFT_RIGHT, _leftRightServoAngle);
    }


    void color_led_pwm(int red, int green, int blue)
    {
        lgTxPwm(_chip.getHandle(), _options.led_red_pin, 1000, 100 * red / 255.0, 0, 0);
        lgTxPwm(_chip.getHandle(), _options.led_green_pin, 1000, 100 * green / 255.0, 0, 0);
        lgTxPwm(_chip.getHandle(), _options.led_blue_pin, 1000, 100 * blue / 255.0, 0, 0);
    }

    void whistle()
    {
        lgGpioWrite(_chip.getHandle(), _options.buzzer_pin, 0);
        usleep(1000000 / 10);
        lgGpioWrite(_chip.getHandle(), _options.buzzer_pin, 1);
        usleep(1000000);

        lgGpioWrite(_chip.getHandle(), _options.buzzer_pin, 0);
        usleep(2000000 / 10);
        lgGpioWrite(_chip.getHandle(), _options.buzzer_pin, 1);
        usleep(2000000);
    }

    void moveForward()
    {
        lgGpioWrite(_chip.getHandle(), _options.right_motor_go, 1);
        lgGpioWrite(_chip.getHandle(), _options.right_motor_back, 0);

        lgGpioWrite(_chip.getHandle(), _options.left_motor_go, 1);
        lgGpioWrite(_chip.getHandle(), _options.left_motor_back, 0);
        setSpeed(_speed);
    }

    void moveBack()
    {
        lgGpioWrite(_chip.getHandle(), _options.right_motor_go, 0);
        lgGpioWrite(_chip.getHandle(), _options.right_motor_back, 1);

        lgGpioWrite(_chip.getHandle(), _options.left_motor_go, 0);
        lgGpioWrite(_chip.getHandle(), _options.left_motor_back, 1);
        setSpeed(_speed);
    }

    void moveLeft()
    {
        lgGpioWrite(_chip.getHandle(), _options.right_motor_go, 1);
        lgGpioWrite(_chip.getHandle(), _options.right_motor_back, 0);

        lgGpioWrite(_chip.getHandle(), _options.left_motor_go, 0);
        lgGpioWrite(_chip.getHandle(), _options.left_motor_back, 0);
        setSpeed(_speed);
    }

    void spinLeft()
    {
        lgGpioWrite(_chip.getHandle(), _options.right_motor_go, 1);
        lgGpioWrite(_chip.getHandle(), _options.right_motor_back, 0);

        lgGpioWrite(_chip.getHandle(), _options.left_motor_go, 0);
        lgGpioWrite(_chip.getHandle(), _options.left_motor_back, 1);
        setSpeed(_speed);
    }

    void moveRight()
    {
        lgGpioWrite(_chip.getHandle(), _options.right_motor_go, 0);
        lgGpioWrite(_chip.getHandle(), _options.right_motor_back, 0);

        lgGpioWrite(_chip.getHandle(), _options.left_motor_go, 1);
        lgGpioWrite(_chip.getHandle(), _options.left_motor_back, 0);
        setSpeed(_speed);
    }

    void spinRight()
    {
        lgGpioWrite(_chip.getHandle(), _options.right_motor_go, 0);
        lgGpioWrite(_chip.getHandle(), _options.right_motor_back, 1);

        lgGpioWrite(_chip.getHandle(), _options.left_motor_go, 1);
        lgGpioWrite(_chip.getHandle(), _options.left_motor_back, 0);
        setSpeed(_speed);
    }

    void stop()
    {
        lgGpioWrite(_chip.getHandle(), _options.right_motor_go, 0);
        lgGpioWrite(_chip.getHandle(), _options.right_motor_back, 0);

        lgGpioWrite(_chip.getHandle(), _options.left_motor_go, 0);
        lgGpioWrite(_chip.getHandle(), _options.left_motor_back, 0);
        setSpeed(_speed);
    }

    int getSpeed()
    {
        return _speed;
    }
    
    void setSpeed(int speed)
    {
        if(speed > 100)
            speed = 100;
        if(speed < 20)
            speed = 20;
        _speed = speed;
        lgTxPwm(_chip.getHandle(), _options.left_motor_pwm, 2000, _speed, 0, 0);
        lgTxPwm(_chip.getHandle(), _options.right_motor_pwm, 2000, _speed, 0, 0);
    }

    ~G1Tank()
    {
        printf("destructing tank\n");
        printf("waiting for tank threads to finish\n");
        _stopThreads = true;
        for(auto& thread: _threads)
        {
            thread.join();
        }
        printf("tank threads finished\n");
    }

    enum Servo { FRONT, LEFT_RIGHT, UP_DOWN };

    enum ServoControl { eFRONT_LEFT = 1, eFRONT_RIGHT, eUP_DOWN_UP, eUP_DOWN_DOWN, eLEFT_RIGHT_RESET, eLEFT_RIGHT_LEFT, eLEFT_RIGHT_RIGHT, eUP_DOWN_RESET };

    bool setSocketBlockingEnabled(int fd, bool blocking)
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

    float getServoAngle(Servo servo){
        switch(servo)
        {
            case FRONT: return _frontServoAngle;
            case LEFT_RIGHT: return _leftRightServoAngle;
            default: return _upDownServoAngle;
        }

    }


     void setServoAngle(Servo servo, float angle)
    {
        // if(angle < 0 || angle > 180)
        // {
        //      throw std::invalid_argument("Servo angle must be in range [0-180] degrees");
        // }
        float prev_angle = this->getServoAngle(servo);
        if(angle < 0) angle = 0;
        if(angle > 180) angle = 180;
        if(angle < 45 && servo == Servo::UP_DOWN) angle = 45;
        int gpio = 11;
        switch(servo)
        {
            case FRONT: gpio = _options.front_servo_pin; this->_frontServoAngle = angle; break;
            case LEFT_RIGHT: gpio = _options.lef_right_servo_pin; this->_leftRightServoAngle = angle; break;
            case UP_DOWN: gpio = _options.up_down_servo_pin; this->_upDownServoAngle = angle; break;

        }

        // convert angle to pulse width between 0.5ms and 2.5ms
        long pulse_width = 500000 + angle / 180 * 2000000;  // 500000 - 2500000 nanoseconds


        // generate enough software PWM pulses at 50Hz, period = 20000µs = 20ms
        auto cycles = 18;
        if(std::abs(prev_angle - angle) <= 2.1) cycles = 1;
        for(int i = 0; i <= cycles; i++)
        {
            lgGpioWrite(_chip.getHandle(), gpio, 1);
            timespec remaining, request = { 0, pulse_width }; //nanoseocnds
            nanosleep(&request, &remaining);
            lgGpioWrite(_chip.getHandle(), gpio, 0);
            request.tv_nsec = (20000000 - pulse_width);
            nanosleep(&request, &remaining);
        }

        
        // printf("Set servo angle=%.2f\n", angle);
        // // int servoCycles = 0; //infinite
        // // lgTxServo(_chip.getHandle(), gpio, pulse_width * 1000, 50, 0, servoCycles);

        // int servoCycles = 18; //infinite
        // lgTxServo(_chip.getHandle(), gpio, pulse_width * 1000, 50, 0, servoCycles);
        // lguSleep(0.6);
        
     
        // while (int isBusy = lgTxBusy(_chip.getHandle(), gpio, LG_TX_PWM))
        // {
        //     auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            
        //     // std::time_t time = std::time({});
        //     // https://en.cppreference.com/w/cpp/chrono/c/strftime
        //     char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
        //     std::strftime(std::data(timeString), std::size(timeString),
        //                 "%FT%TZ", std::gmtime(&time));
        //     // std::cout << timeString << '\n';
        //     // printf("%s, isBusy %d\n", std::to_string(givemetime).c_str(), isBusy);            
        //      printf("%s, isBusy %d\n", timeString, isBusy);
        //      lguSleep(0.01);
        // }
       
       
        
    }
    // void setServoAngle(Servo servo, float angle)
    // {
    //     // if(angle < 0 || angle > 180)
    //     // {
    //     //      throw std::invalid_argument("Servo angle must be in range [0-180] degrees");
    //     // }
    //     if(angle < 0) angle = 0;
    //     if(angle > 180) angle = 180;
    //     if(angle < 45 && servo == Servo::UP_DOWN) angle = 45;
    //     int gpio = 11;
    //     switch(servo)
    //     {
    //         case FRONT: gpio = _options.front_servo_pin; this->_frontServoAngle = angle; break;
    //         case LEFT_RIGHT: gpio = _options.lef_right_servo_pin; this->_leftRightServoAngle = angle; break;
    //         case UP_DOWN: gpio = _options.up_down_servo_pin; this->_upDownServoAngle = angle; break;

    //     }
       
        
    //     float pulse_width = 0.5 + angle / 180 * 2;
    
       
    //     // printf("%.2f\n", pulse_width * 1000);
    //     int servoCycles = 18;
    //     //  lgTxServo(_chip.getHandle(), gpio, pulse_width * 1000, 50, 0, 0);
    //     lgTxServo(_chip.getHandle(), gpio, pulse_width * 1000, 50, 0, servoCycles);
    //     //lguSleep(0.2);
    //     // double start, end;
    //     // start = lguTime();
    //     // // wait until pulses are processed
    //     while (lgTxBusy(_chip.getHandle(), gpio, LG_TX_PWM)) lguSleep(0.01);
    //     // //lguSleep(0.01);
    //     // printf("angle set");
    //     // end = lguTime();

    //     // printf("%d cycles at 50 Hz took %.2f seconds\n", servoCycles, end-start);
    // }
    void startReceive(thread_info& ti) {
        _threads.emplace_back(&G1Tank::TCP_receive, this, std::ref(ti));
    }
    void TCP_receive(thread_info& ti);
    void parseCommand(const std::string& command);
private:
  
    
    bool _stopThreads = false;
    int _speed = { 80 };
    float _frontServoAngle = {90};
    float _upDownServoAngle = {90};
    float _leftRightServoAngle = {90};
    std::vector<std::thread> _threads = {};
    G1Tank::Chip _chip;
    G1Tank::Options _options;
};