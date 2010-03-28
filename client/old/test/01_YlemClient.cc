#include <iostream>	
#include "01_YlemClient.h"
#include "YlemClient.h"
using namespace::ylem;

CPPUNIT_TEST_SUITE_REGISTRATION( YlemClientTest/*此处写入单元测试类名*/);
#define HOST "h08-vm02.corp.cnb.yahoo.com"
#define PORT 7001
#define DOMAIN "test"
#define CLASS ""

void YlemClientTest::setUp()
{
	// 记录测试开始时刻以便输出耗时信息
	gettimeofday(&begin,0);

	// TODO: 可以在本函数中插入所有测试用例共用的资源初始化代码
}

void YlemClientTest::tearDown()
{
	struct timeval end;
	double elapsed;

	// 单次测试结束，根据当前时刻输出耗时信息
	gettimeofday(&end,0);
	elapsed=(end.tv_sec-begin.tv_sec)+(end.tv_usec-begin.tv_usec)/1000000.0;
	std::cerr<<" ["<<elapsed<<" s] ";
}

void YlemClientTest::ylem_connect()
{
	YlemClient ylem;

	CPPUNIT_ASSERT(ylem.ylem_connect(HOST,PORT,DOMAIN,CLASS));
	CPPUNIT_ASSERT(ylem.ylem_connect(HOST,PORT,DOMAIN,""));
	CPPUNIT_ASSERT(ylem.ylem_connect(HOST,PORT,"",""));
}

void YlemClientTest::ylem_putfile()
{
	YlemClient ylem;
	CPPUNIT_ASSERT(ylem.ylem_connect(HOST,PORT,DOMAIN,CLASS));
	YlemState state = ylem.ylem_putfile("ylemClientTestfile",__FILE__);
	CPPUNIT_ASSERT_EQUAL(state,YLEM_OK);

}
void YlemClientTest::ylem_getpaths()
{
	YlemClient ylem;
	CPPUNIT_ASSERT(ylem.ylem_connect(HOST,PORT,DOMAIN,CLASS));
	YlemState state = ylem.ylem_putfile("mykey",__FILE__);
	CPPUNIT_ASSERT_EQUAL(state,YLEM_OK);
	std::string str,mimetype;
	state = ylem.ylem_getpaths(str,mimetype,"mykey");
	std::cout << str << endl;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(str,state,YLEM_OK);

}
void YlemClientTest::ylem_delete()
{
	YlemClient ylem;
	CPPUNIT_ASSERT(ylem.ylem_connect(HOST,PORT,DOMAIN,CLASS));
	YlemState state = ylem.ylem_putfile("ylem_delete",__FILE__);
	CPPUNIT_ASSERT_EQUAL(state,YLEM_OK);
	state = ylem.ylem_delete("ylem_delete");
	CPPUNIT_ASSERT_EQUAL(state,YLEM_OK);
}
void YlemClientTest::ylem_listfids()
{
	YlemClient ylem;
	CPPUNIT_ASSERT(ylem.ylem_connect(HOST,PORT,DOMAIN,CLASS));
	std::string str;
	YlemState state = ylem.ylem_listfids(str,0,100);
	std::cout << str << endl;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(str,state,YLEM_OK);
}
void YlemClientTest::ylem_listkeys()
{
	YlemClient ylem;
	CPPUNIT_ASSERT(ylem.ylem_connect(HOST,PORT,DOMAIN,CLASS));
	std::string str;
	YlemState state = ylem.ylem_listkeys(str,"","",100);
	std::cout << str << endl;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(str,state,YLEM_OK);
}
