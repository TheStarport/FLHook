#include "PCH.hpp"
#include "Exceptions/ErrorInfo.hpp"

std::wstring_view ErrorInfo::GetText(const Error err)
{
	if (const auto errInfo = Errors.find(err); errInfo != Errors.end())
	{
		return errInfo->second;
	}

	return L"No error text available";
}
