
#include "std.h"
#include "utils.h"

class TestClass2
{
public:
	TestClass2()
	{
		printk("TestClass2 Constructor!\n");
	}
	void Show(char *msg)
	{
		printk(msg);
	}
};

TestClass2 gt12, gt22;

void Test2()
{
	TestClass2 t;
	t.Show("What is your name2?\n");
}
