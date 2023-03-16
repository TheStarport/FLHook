#include "PCH.hpp"
#include "Exceptions/ErrorInfo.hpp"

std::string ErrorInfo::GetText(const Error err) const
{
	if (const auto errInfo = errors.find(err); errInfo != errors.end())
	{
		return errInfo->second;
	}

	return "No error text available";
}
