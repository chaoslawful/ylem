/*
 * =====================================================================================
 * 
 *       Filename:  Config.h 
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年04月16日 14时25分36秒 CST
 *       Revision:  none * 
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  CONFIG_H_
#define  CONFIG_H_

#ifndef BASE_H_
#include "Base.h"
#endif
#include <yconfig/yConfig.h>
#include <memory>
using namespace::std;

namespace ylem{

	/*
	 * =====================================================================================
	 *        Class: Config
	 *  Description: ylem所需配置文件
	 * =====================================================================================
	 */
	class Config{
	public:
		static Config& getInstance();
		bool getBaseConfig(string& out,const string& key) const;
		bool getBaseConfig(int& out,const string& key) const;
	protected:
		Config();
		~Config();
		Config(const Config&);
		Config& operator=(const Config&);
		inline void _OpenConfigfile(const string& file);
	protected:
		friend class auto_ptr<Config>;
		static auto_ptr<Config> _instance;
		mapss _BaseConfig;
		yConfigFile _cfg;
	};
}


#endif   /* ----- #ifndef CONFIG_H_  ----- */

