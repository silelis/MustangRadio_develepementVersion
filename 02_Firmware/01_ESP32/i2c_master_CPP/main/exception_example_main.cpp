/* C++ exception handling example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <iostream>

using std::cout;
using std::endl;
using std::runtime_error;





//#include "i2c_master/my_i2c_master.h"
#include "MCP23008/MCP23008.h"




/* A simple class which may throw an exception from constructor */
class Throwing
{
public:
    Throwing(int arg)
    : m_arg(arg)
    {
        cout << "In constructor, arg=" << arg << endl;
        if (arg == 0) {
            throw runtime_error("Exception in constructor");
        }
    }

    ~Throwing()
    {
        cout << "In destructor, m_arg=" << m_arg << endl;
    }

protected:
    int m_arg;
};

/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void)
{
    cout << "app_main starting" << endl;

	
	//my_i2c_master *pmy_master;
	//pmy_master = new my_i2c_master();
	//pmy_master->i2cPing(0b0100000);
	
	MCP23008* pMCP23008;
	pMCP23008 = new MCP23008(MCP23008_I2C_DEVICE_OPCODE);	
	
	
	
	
	while (true)
	{
	         
	}
	
    try {
        /* This will succeed */
        Throwing obj1(42);

        /* This will throw an exception */
        Throwing obj2(0);

        cout << "This will not be printed" << endl;
    } catch (const runtime_error &e) {
        cout << "Exception caught: " << e.what() << endl;
    }

    cout << "app_main done" << endl;
}
