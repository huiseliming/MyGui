#pragma once
#include "EasyGuiForward.h"
#include <vulkan/vulkan.h>

#define VK_ASSERT_SUCCESSED(Expression)                   \
    if (VkResult vk_result = (Expression)) [[unlikely]] { \
        throw VkException(__LINE__, __FILE__, vk_result); \
    }                                                     \

const char* VkResultToString(VkResult vk_result);

class VkException : public std::runtime_error
{

public:
	VkException(int line, const char* file, VkResult vk_result) noexcept
		: std::runtime_error("")
		, _Line(line)
		, _File(file)
		, _VkResult(vk_result)
	{}

	const char* what() const noexcept override
	{
		std::ostringstream oss;
		oss << "[" << _File << "(" << _Line << ")] ASSERT  <VkResult:" << VkResultToString(_VkResult) << "(" << _VkResult <<")> FAILED";
		_WhatBuffer = oss.str();
		return _WhatBuffer.c_str();
	}

	virtual const char* GetType() const noexcept
	{
		return "VkException";
	}

private:
	int32_t _Line;
	std::string _File;
	VkResult _VkResult;

private:
	mutable std::string _WhatBuffer;
};
