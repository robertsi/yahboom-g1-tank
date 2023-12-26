#include <iostream>
#include <sstream>
#include <mutex>
#include <map>
// #include <ostream>




namespace gpiozero _GLIBCXX_VISIBILITY(default)
{
      void sayHello()
      {
            std::cout << "Hi from gpiozero\n";
      }

      class Factory {
            public:
                  void reserve_pins() {
                        {
                              {
                                    const std::lock_guard<std::mutex> lock(_lock);
                                    //dp stuff safe
                              }
                        }
                  };
                  void release_pins() {};
                  void release_all() {};

                  void reserve_pins_virt() { _reserve_pins(); };
                  virtual ~Factory() = default;
                  std::mutex _lock;
            private:
                  virtual void _reserve_pins() = 0;
                  std::map<char, char> _reservations;
                
      };

      class Pin {
            public:
                  // operator std::string () const {
                  //       return "Pin";
                  // }
                  std::string toString() const { return _toString(); }
            private:
                  virtual std::string _toString() const = 0;
                  virtual ~Pin() = 0;
      };

      std::ostream& operator << (std::ostream &os, const gpiozero::Pin &pin) {
            return (os << pin.toString());
      }

      std::ostream& operator << (std::ostream &os, const gpiozero::Pin *pin) {
                  return (os << pin->toString());
      }

      std::string Pin::_toString() const { 
            std::stringstream ss;
            ss << "Pin" << std::endl;
            return ss.str();
      }

      class PiPin : public Pin {
            public:
                  // operator std::string () const {
                  //       return "Pin";
                  // }
                  std::string _toString() const override { 
                        std::stringstream ss;
                        ss << "PiPin";
                        return ss.str();
                  }
            private:
                  virtual ~PiPin() = 0;
      };



      class PiFactory {
            
      };

   
}

