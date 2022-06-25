#pragma once
#include <any>
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>

namespace Reflect
{
	class Record
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

	protected:
		//int64_t _Id{ 0 };
		std::string _Name;
		std::map<std::string, std::any> _AttributeMap;
	};

}
