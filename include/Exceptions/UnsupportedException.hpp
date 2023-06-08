#pragma once

class DLL UnsupportedException final : std::exception
{
  public:
	explicit UnsupportedException() {}
	~UnsupportedException() noexcept override = default;
	[[nodiscard]] char const* what() const override { return "The action performed is not supported or valid."; }
};