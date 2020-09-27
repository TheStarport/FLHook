#ifndef __MAIL_H__
#define __MAIL_H__ 1

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>

namespace Mail
{
	int MailCountUnread(const std::wstring &wscCharname, const std::string &scExtension);
	int MailCount(const std::wstring &wscCharname, const std::string &scExtension);
	void MailShow(const std::wstring &wscCharname, const std::string &scExtension, int iFirstMsg);
	bool MailDel(const std::wstring &wscCharname, const std::string &scExtension, int iMsg);
	void MailCheckLog(const std::wstring &wscCharname, const std::string &scExtension);
	bool MailSend(const std::wstring &wscCharname, const std::string &scExtension, const std::wstring &wscMsg);
}

#endif
