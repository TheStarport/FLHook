#ifndef __MAIL_H__
#define __MAIL_H__ 1

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>

namespace Mail
{
	int MailCountUnread(const wstring &wscCharname, const string &scExtension);
	int MailCount(const wstring &wscCharname, const string &scExtension);
	void MailShow(const wstring &wscCharname, const string &scExtension, int iFirstMsg);
	bool MailDel(const wstring &wscCharname, const string &scExtension, int iMsg);
	void MailCheckLog(const wstring &wscCharname, const string &scExtension);
	bool MailSend(const wstring &wscCharname, const string &scExtension, const wstring &wscMsg);
}

#endif