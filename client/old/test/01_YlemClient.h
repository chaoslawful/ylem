#ifndef YLEMCLIENT_H_STUB
#define YLEMCLIENT_H_STUB

#include <cppunit/extensions/HelperMacros.h>
// 为了得到 timeval 结构和 gettimeofday() 函数的声明，以便输出每次测试的耗时
#include <sys/time.h>

class YlemClientTest
	: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( YlemClientTest /* 这里是单元测试类名 */ );
	CPPUNIT_TEST(ylem_connect /* 单个测试用例函数 */ );
	CPPUNIT_TEST(ylem_putfile/* 单个测试用例函数 */ );
	CPPUNIT_TEST(ylem_getpaths /* 单个测试用例函数 */ );
	CPPUNIT_TEST(ylem_delete /* 单个测试用例函数 */ );
	CPPUNIT_TEST(ylem_listfids /* 单个测试用例函数 */ );
	CPPUNIT_TEST(ylem_listkeys /* 单个测试用例函数 */ );
	// TODO: 这里可以用 CPPUNIT_TEST() 宏继续增加测试用例函数
	CPPUNIT_TEST_SUITE_END();
//	CPPUNIT_TEST_SUITE_REGISTRATION(YlemClientTest);

	struct timeval begin; // 为了计算测试耗时而定义

	// TODO: 这里可以增加测试用例都要使用的变量，注意变量值并不在用例之间共享
public:
	void setUp();
	void tearDown();

	/* 测试用例函数声明 */
	void ylem_connect();
	void ylem_putfile();
	void ylem_getpaths();
	void ylem_delete();
	void ylem_listfids();
	void ylem_listkeys();
	// TODO: 这里可以继续增加测试用例函数声明，所有测试用例函数的原型相同
};

#endif

