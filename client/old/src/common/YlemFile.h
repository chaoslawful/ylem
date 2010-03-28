/*
 * =====================================================================================
 * 
 *       Filename:  YlemFile.h 
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年04月17日 11时20分26秒 CST
 *       Revision:  none * 
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  YLEMFILE_H_
#define  YLEMFILE_H_

#ifndef BASE_H_
#include "Base.h"
#endif

namespace ylem{

/*
 * =====================================================================================
 *        Class: YlemFile
 *  Description: Ylem内部文件信息类 
 * =====================================================================================
 */
	class YlemFile
	{
		public:
			// 获取可能存储位置总数
			int get_dev_count() const;
			// 获取文件内部id
			string get_fid() const;
			// 获取给定下标处的存储设备id
			int get_devid(int idx) const;
			// 获取给定下标处的存储位置URL
			string get_path(int idx) const;
			// 选定一个下标处的存储位置作为实际使用的位置
			int select_dev(int idx);
			// 构造函数初始化_idx为0
			YlemFile():_idx(0){};
			operator mapss() const;
			YlemFile& operator=(const mapss& info);
		private:
			mapss _info;		// 记录Ylem内部文件信息 
								// 记录关闭文件文件信息
			mutable	mapss _closefile;
			int _idx;			// 记录Ylem存储位置下标(默认为0)
	};
}

#endif   /* ----- #ifndef YLEMFILE_H_  ----- */

