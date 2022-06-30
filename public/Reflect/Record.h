#pragma once
#include <any>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <cassert>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <typeindex>

#include "MyGuiExport.h"


#ifdef __RUN_CODE_GENERATOR__
#define METADATA(...)  [[clang::annotate("meta" __VA_OPT__(", ") #__VA_ARGS__)]]// __attribute__((annotate("meta" __VA_OPT__(", ") #__VA_ARGS__)))
#define CLASS(...)     METADATA(__VA_ARGS__)
#define STRUCT(...)    METADATA(__VA_ARGS__)
#define ENUM(...)      METADATA(__VA_ARGS__)
#define FIELD(...)	   METADATA(__VA_ARGS__)
#define FUNCTION(...)  METADATA(__VA_ARGS__)
#else
#define CLASS(...)
#define ENUM(...)
#define FIELD(...)
#define FUNCTION(...)
#define METADATA(...)
#endif


namespace Reflect
{

	class MYGUI_API Record
	{
	public:
		Record(const std::string& name = "") : _Name(name) {}
		virtual ~Record() = default;

		Record(const Record&) = delete;
		Record& operator=(const Record&) = delete;
		Record(Record&&) = delete;
		Record& operator=(Record&&) = delete;

		//int64_t GetId() { return _Id; }
		const std::string& GetName() { return _Name; }
		bool HasAttribute(const std::string& attribute_name) { return _AttributeMap.contains(attribute_name); }
		std::any GetAttributeValue(const std::string& attribute_name)
		{
			auto it = _AttributeMap.find(attribute_name);
			if (it != _AttributeMap.end())
			{
				return it->second;
			}
			return std::any();
		}
		template<typename T>
		T GetAttributeValueAs(const std::string& attribute_name)
		{
			std::any attribute_any = GetAttributeValue(attribute_name);
			if (attribute_any.has_value())
			{
				try
				{
					return std::any_cast<T>(attribute_any);
				}
				catch (const std::bad_any_cast& e)
				{
					return T();
				}
			}
			return T();
		}
	protected:
		//int64_t _Id{ 0 };
		std::string _Name;
		std::map<std::string, std::any> _AttributeMap;
	private:
		template<typename T> friend struct TCustomClassModifier;
		template<typename T> friend struct TDefaultClassInitializer;
	};

}
