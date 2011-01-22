#include "std.h"
#include "utils.h"
#include <vector>
using namespace std;

class TestClass
{
public:
	TestClass()
	{
		printk("TestClass Constructor!\n");
	}
	void Show(const char *msg)
	{
		printk(msg);
	}
private:
	u32 name;
};

TestClass gt1, gt2;

extern "C"{
void Test();
}

void Test()
{
	TestClass t;
	TestClass *pt = new TestClass[6];
	
	pt[0].Show("What is your name?\n");
	delete []pt;
}
