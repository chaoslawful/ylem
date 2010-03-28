/*
 * =====================================================================================
 *
 *       Filename:  YlemClient.cc
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月25日 13时54分08秒 CST
 *       Revision:  none *
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 *
 * =====================================================================================
 */

#include <yaxx/StringUtils.h>
#include <iostream>
using namespace::std;

#include "YlemClient.h"
using namespace::ylem;

//#define FILENAME "/tmp/file/test.dat"
#define FILENAME "/tmp/file/1.8m.JPG"

/* 上传文件 */
void putfile(const string& key,const string& filename)
{
	YlemClient client;
	bool b=client.ylem_connect("ds1.platform.yahoo.com",7001,"myDomain","my_class");
	if(b)
	{
		// 上传文件
		//		YlemState state = client.ylem_putfile("uggwy.txt","/tmp/file/2m.JPG");
		YlemState state = client.ylem_putfile(key, filename);
		if(state == YLEM_OK)
			cout << key << " -- OK " << endl;
		else
			cout << key << " -- fail" << state << endl;
	}
	else
	{
		cout << "connect failure\n";
	}
}
/* 删除所有文件 */
void deletefile(const string &key)
{
	YlemClient client;
	bool b=client.ylem_connect("ds1.platform.yahoo.com",7001,"myDomain","my_class");
	if(!b)
	{
		cout << "connect failure\n";
		return;
	}

	if(key.size() != 0)
	{
		YlemState state = client.ylem_delete(key);
		if(state == YLEM_OK){
			cout << "delete file " << key << "\n";
		}else{
			cout << "not delete file " << key << "\n";
		}
	}else{
		while(1)
		{
			// 枚举前1000条文件
			mapss Data;
			YlemState state = client.ylem_listkeys(Data,"","",1000);
			if(state == YLEM_OK)
			{
				mapssor it=Data.find("key_count");
				int iKeyCount=0;
				if(it != Data.end())
				{
					iKeyCount = YaxxStringUtils::toInt((*it).second);
					cout << iKeyCount << "\n";
				}
				ostringstream oss;
				for(int i =1; i<iKeyCount+1; ++i)
				{
					oss.str("");
					oss << "key_" << i;
					it = Data.find(oss.str());
					if(it == Data.end())
						continue;
					string strfile = (*it).second;
					state = client.ylem_delete(strfile);
					if(state == YLEM_OK){
						cout << "delete file " << strfile << "\n";
					}else{
						cout << "not delete file" << strfile << "\n";
					}
				}	
			}else{
				break;
			}
		}
	}
}

// 获取指定的key
void getkeys(const string& key)
{
	YlemClient client;
	bool b=client.ylem_connect("ds1.platform.yahoo.com",7001,"myDomain","my_class");
	if(b){
		mapss mdata;
		YlemState state = client.ylem_listkeys(mdata,key);
		if(state == YLEM_OK)
		{
			cout << " key  " << " value\n";
			for(mapssor it=mdata.begin(); it != mdata.end(); ++it)
			{
				cout << it->first << ":" << it->second << "\n";
			}
		}else{
			cout << "not found key: " << key << "\n";
		}
	}
	else{
		cout << "connect failure\n";
	}
}

// 列出部分keys
void listkeys(string str="",int limit =100)
{
	YlemClient client;
	bool b=client.ylem_connect("ds1.platform.yahoo.com",7001,"myDomain","my_class");
	if(!b){
		cout << "connect err\n";
		return;
	}
	// 枚举前1000条文件
	mapss Data;
	YlemState state = client.ylem_listkeys(Data,str,"",limit);
	if(state == YLEM_OK)
	{
		mapssor it=Data.find("key_count");
		int iKeyCount=0;
		if(it != Data.end())
		{
			iKeyCount = YaxxStringUtils::toInt((*it).second);
		}
		ostringstream oss;
		for(int i =1; i<iKeyCount+1; ++i)
		{
			oss.str("");
			oss << "key_" << i;
			it = Data.find(oss.str());
			if(it != Data.end())
			{
				cout <<  it->second << "\n";
			}
		}
	}
}

// 获取getpaths
void getpaths(const string& key)
{
	YlemClient client;
	bool b=client.ylem_connect("ds1.platform.yahoo.com",7001,"myDomain","my_class");
	if(b){
		mapss mdata;
		string mimetype;
		YlemState state = client.ylem_getpaths(mdata,mimetype,key);
		if(state == YLEM_OK)
		{
			cout << "found key : " << key << "\n";
			for(mapssor it=mdata.begin(); it != mdata.end(); ++it)
			{
				cout << "    " <<it->first << ":" << it->second << "\n";
			}
		}else{
			cout << "not found key: " << key << "\n";
		}
	}
	else{
		cout << "connect failure\n";
	}
}


int
main ( int argc, char *argv[] )
{

	if(argc < 2)
	{
		cout << "sudo ./ylemClient method [key] [filepath] [num]\n";
		cout << "method: ylemClient support 4 methods\n";
		cout << "		delete: delete a key \n";
		cout << "		put: put file to ylemServer\n";
		cout << "		getpaths: get file paths\n";
		cout << "		getkeys: get keys\n";
		cout << "key: that key is string \n";
		cout << "filepath and num only to put an effective method\n";
		return 0;
	}else{
		string strMethod = YaxxStringUtils::toLower(argv[1]);
		if(strMethod == "delete")		// delete 删除文件操作 
		{
			string strKey;
			if(argc > 2)
				strKey = argv[2];
			deletefile(strKey);
		}else if(strMethod == "createopen"){ // createopen 操作
			YlemClient client;
			client.ylem_connect("ds1.platform.yahoo.com",7001,"myDomain","my_class");
			YlemFile file;
			cout << "test ylem_createopen()\n";
			YlemState state = client.ylem_createopen(file,"abc","",true,123);	
			if(state == YLEM_OK)
			{
				cout << "ylem_createopen ok\n";
				mapss mData = file;
				for(mapssor it = mData.begin(); it != mData.end(); ++it)
				{
					cout << "key:value == "<< it->first << ":" << it->second << endl;
				}
				file.select_dev(1);
				mData = file;
				for(mapssor it = mData.begin(); it != mData.end(); ++it)
				{
					cout << "key:value == "<< it->first << ":" << it->second << endl;
				}
				cout << "test ylem_createclose()\n";
				state = client.ylem_createclose(file);
				if(state == YLEM_OK)
					cout << "ylem_createclose() ok\n";
				else 
					cout << "ylem_createclose() fail\n";
			}else{
				cout << "ylem_createopen fail\n";
			}
		}else if(strMethod == "put"){	// put 上传文件操作
			if(argc < 4){
				cout << "put method err\n";
				return 1;
			}
			string strKey = argv[2];
			string strFilePath = argv[3];
			int iNum=0;
			if(argc == 5)			
				iNum = YaxxStringUtils::toInt(argv[4]);
			if(iNum == 0)
				putfile(strKey,strFilePath);
			else{
				for(int i=0; i < iNum; ++i)
				{
					string key = strKey;
					ostringstream oss;
					oss << key << i;
					putfile(oss.str(),strFilePath);
				}
			}
		}else if(strMethod == "getpaths"){
			if(argc < 3){
				cout << "method getpaths err\n";
				return 1;
			}
			int iNum=0;
			if(argc == 4)
				iNum = YaxxStringUtils::toInt(argv[3]);
			if(iNum == 0)
				getpaths(argv[2]);
			else{
				for(int i =0; i < iNum; ++i)
				{
					ostringstream oss;
					oss << argv[2] << i;
					getpaths(oss.str());
				}
			}
		}else if (strMethod == "getkeys"){
			if(argc < 3){
				cout << "method getkeys err\n";
				return 1;
			}
			getkeys(argv[2]);	
		}else{
			cout << "list some keys\n";
			listkeys("",10);
			cout << strMethod << " no support method\n";
		}
	}
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

