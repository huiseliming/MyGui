#pragma once
#include "Record.h"
#include <spdlog/spdlog.h>

namespace Reflect
{
    class Enum;
    class Class;

    enum ECastTypeFlagBits : uint32_t
    {
        CTFB_NoFlag = 0ULL,
        CTFB_VoidBit = 1ULL << 0,
        CTFB_BoolBit = 1ULL << 1,
        CTFB_FloatBit = 1ULL << 2,
        CTFB_DoubleBit = 1ULL << 3,
        CTFB_SInt8Bit = 1ULL << 4,
        CTFB_SInt16Bit = 1ULL << 5,
        CTFB_SInt32Bit = 1ULL << 6,
        CTFB_SInt64Bit = 1ULL << 7,
        CTFB_UInt8Bit = 1ULL << 8,
        CTFB_UInt16Bit = 1ULL << 9,
        CTFB_UInt32Bit = 1ULL << 10,
        CTFB_UInt64Bit = 1ULL << 11,
        CTFB_StringBit = 1ULL << 12,
        CTFB_EnumBit = 1ULL << 13,
        CTFB_ClassBit = 1ULL << 14,
        CTFB_VectorBit = 1ULL << 16,
        CTFB_PtrMapBit = 1ULL << 17,
        CTFB_Int64MapBit = 1ULL << 18,
        CTFB_StringMapBit = 1ULL << 19,
        CTFB_PtrUnorderedMapBit = 1ULL << 20,
        CTFB_Int64UnorderedMapBit = 1ULL << 21,
        CTFB_StringUnorderedMapBit = 1ULL << 22,

        CTFB_SIntBits = CTFB_SInt8Bit | CTFB_SInt16Bit | CTFB_SInt32Bit | CTFB_SInt64Bit,
        CTFB_UIntBits = CTFB_UInt8Bit | CTFB_UInt16Bit | CTFB_UInt32Bit | CTFB_UInt64Bit,
        CTFB_IntBits = CTFB_SIntBits | CTFB_UIntBits,
        CTFB_FloatBits = CTFB_FloatBit | CTFB_DoubleBit,

        CTFB_PointerBit = 1ULL << 31,
    };

    template<typename T> struct TStaticBuiltinTypeFlag    { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_NoFlag; };
    template<> struct TStaticBuiltinTypeFlag<bool>        { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_BoolBit;   };
    template<> struct TStaticBuiltinTypeFlag<int8_t>      { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_SInt8Bit;  };
    template<> struct TStaticBuiltinTypeFlag<int16_t>     { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_SInt16Bit; };
    template<> struct TStaticBuiltinTypeFlag<int32_t>     { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_SInt32Bit; };
    template<> struct TStaticBuiltinTypeFlag<int64_t>     { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_SInt64Bit; };
    template<> struct TStaticBuiltinTypeFlag<uint8_t>     { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_UInt8Bit;  };
    template<> struct TStaticBuiltinTypeFlag<uint16_t>    { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_UInt16Bit; };
    template<> struct TStaticBuiltinTypeFlag<uint32_t>    { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_UInt32Bit; };
    template<> struct TStaticBuiltinTypeFlag<uint64_t>    { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_UInt64Bit; };
    template<> struct TStaticBuiltinTypeFlag<float>       { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_FloatBit;  };
    template<> struct TStaticBuiltinTypeFlag<double>      { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_DoubleBit; };
    template<> struct TStaticBuiltinTypeFlag<std::string> { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_StringBit; };

    class MYGUI_API Type : public Record
	{
	public:
		Type(const std::string& name = "") : Record(name) {}

        uint32_t GetCastTypeFlag() const { return _CastTypeFlag; }
        uint32_t GetMemorySize() const { return _MemorySize; }

        void* New()                         { return _New(); }
        void  Delete(void* A)               { _Delete(A); }
        void  Constructor(void* A)          { _Constructor(A); }
        void  Destructor(void* A)           { _Destructor(A); }
        void  CopyAssign(void* A, void* B)  { _CopyAssign(A, B); }
        void  MoveAssign(void* A, void* B)  { _MoveAssign(A, B); }

        virtual Type* GetPointToType() const { return nullptr; }

	protected:
		uint32_t _CastTypeFlag;
		uint32_t _MemorySize;

		void* (*_New)        (            );
		void  (*_Delete)     (void*       );
		void  (*_Constructor)(void*       );
		void  (*_Destructor) (void*       );
		void  (*_CopyAssign) (void*, void*);
		void  (*_MoveAssign) (void*, void*);
    private:
        template<typename T> friend Type* GetType();
	};

    template<typename CppType>
    class TType : public Type
    {
    public:
        TType(const std::string& name = "") : Type(name)
        {
            _MemorySize = sizeof(CppType);
            _New         = []() -> void* { return new CppType(); };
            _Delete      = [](void* A) { delete static_cast<CppType*>(A); };
            _Constructor = [](void* A) { new (A) CppType(); };
            _Destructor  = [](void* A) { ((const CppType*)(A))->~CppType(); };
            _CopyAssign  = [](void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); };
            _MoveAssign  = [](void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); };
        }
    };

    template<typename T>
    class TSimpleStaticType : public TType<T>
    {
    public:
        TSimpleStaticType(const std::string& name = "")
            : TType<T>(name)
        {}
    };

    class PointerType : public Type
    {
        using CppType = void*;
    public:
        PointerType(const std::string& name = "")
            : Type(name)
        {
            _MemorySize = sizeof(CppType);
            _New         = []() -> void* { return new CppType(); };
            _Delete      = [](void* A) { delete static_cast<CppType*>(A); };
            _Constructor = [](void* A) { new (A) CppType(); };
            _Destructor  = [](void* A) { ((const CppType*)(A))->~CppType(); };
            _CopyAssign  = [](void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); };
            _MoveAssign  = [](void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); };
        }

        virtual Type* GetPointToType() const { return _PointToType; }
    private:
        Type* _PointToType{ nullptr };
    private:
        template<typename T> friend Type* GetType();
    };

    template<typename T> Type* GetStaticType() { return nullptr; }
    MYGUI_API template<> Type* GetStaticType<bool>();
    MYGUI_API template<> Type* GetStaticType<float>();
    MYGUI_API template<> Type* GetStaticType<double>();
    MYGUI_API template<> Type* GetStaticType<int8_t>();
    MYGUI_API template<> Type* GetStaticType<int16_t>();
    MYGUI_API template<> Type* GetStaticType<int32_t>();
    MYGUI_API template<> Type* GetStaticType<int64_t>();
    MYGUI_API template<> Type* GetStaticType<uint8_t>();
    MYGUI_API template<> Type* GetStaticType<uint16_t>();
    MYGUI_API template<> Type* GetStaticType<uint32_t>();
    MYGUI_API template<> Type* GetStaticType<uint64_t>();
    MYGUI_API template<> Type* GetStaticType<std::string>();

    MYGUI_API template<> Type* GetStaticType<std::vector<std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::map<void*, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::map<int64_t, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::map<std::string, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::unordered_map<void*, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::unordered_map<int64_t, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::unordered_map<std::string, std::any>>();



    template<typename T> Enum* GetStaticEnum() { return nullptr; }

    MYGUI_API std::unordered_map<std::string, Type*>& GetTypeNameMap();
    MYGUI_API extern std::unordered_map<std::string, Type*>& GTypeNameMap;

    MYGUI_API void AddTypeToNameMap(const std::string& type_name, Type* type_ptr);

    MYGUI_API Enum* FindEnum(const std::string& enum_name);
    MYGUI_API Class* FindClass(const std::string& class_name);

    MYGUI_API std::unordered_map<std::type_index, Type*>& GetTypeIndexMap();
    MYGUI_API extern std::unordered_map<std::type_index, Type*>& GTypeIndexMap;

    MYGUI_API Type* GetType(const std::type_info& type_info);

    // 
    MYGUI_API std::vector<std::unique_ptr<PointerType>>& GetStaticPointerTypes();
    MYGUI_API std::unordered_map<std::type_index, PointerType*>& GetUninitializePointerTypeMap();
    MYGUI_API std::unordered_map<std::type_index, Type*>& GetPointerTypePointToTypeIndexMap();

    template<typename T>
    struct IsReflectClassType
    {
    private:
        template<typename U, Class* (*)()> struct Check {};
        template<class C> static std::true_type Test(Check<C, &C::StaticClass>*);
        template<class C> static std::false_type Test(...);
    public:
        static constexpr bool value = std::is_same<std::true_type, decltype(Test<T>(nullptr))>::value;
    };

    template<typename T>
    constexpr bool ConstexprTrue = true;
    template<typename T>
    constexpr bool ConstexprFalse = false;

    template<typename T>
    Type* GetType()
    {
        static_assert(!std::is_reference_v<T>);
        std::unordered_map<std::type_index, Type*>& type_index_map_ref = GetTypeIndexMap();
        std::type_index type_index = std::type_index(typeid(T));
        auto type_index_map_iterator = type_index_map_ref.find(type_index);
        if (type_index_map_ref.end() != type_index_map_iterator) return type_index_map_iterator->second;
        auto& uninitialize_pointer_type_map_ref = GetUninitializePointerTypeMap();
        auto& pointer_type_point_to_type_index_map_ref = GetPointerTypePointToTypeIndexMap();
        Type* return_type = nullptr;
        if      constexpr (std::is_pointer_v<T>)
        {
            auto& static_pointer_types_ref = GetStaticPointerTypes();
            static_pointer_types_ref.push_back(std::make_unique<PointerType>());
            auto static_pointer_type_ptr = static_pointer_types_ref.back().get();
            auto pointer_type_point_to_type_index_map_iterator = pointer_type_point_to_type_index_map_ref.find(type_index);
            if (pointer_type_point_to_type_index_map_iterator != pointer_type_point_to_type_index_map_ref.end())
            {
                static_pointer_type_ptr->_PointToType = pointer_type_point_to_type_index_map_iterator->second;
            }
            else
            {
                uninitialize_pointer_type_map_ref.insert(std::make_pair(type_index, static_pointer_type_ptr));
            }
            return_type = static_pointer_type_ptr;
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_PointerBit;
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = TStaticBuiltinTypeFlag<T>::Value;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return_type = GetStaticType<std::string>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_StringBit;
        }
        else if constexpr (std::is_class_v<T> && IsReflectClassType<T>::value)
        {
            return_type = T::StaticClass();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_ClassBit;
        }
        else if constexpr (std::is_enum_v<T>)
        {
            return_type = GetStaticEnum<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_EnumBit;
        }
        else if constexpr (std::is_same_v<T, std::vector<std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_VectorBit;
        }
        else if constexpr (std::is_same_v<T, std::map<void*, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_PtrMapBit;
        }
        else if constexpr (std::is_same_v<T, std::map<int64_t, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_Int64MapBit;
        }
        else if constexpr (std::is_same_v<T, std::map<std::string, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_StringMapBit;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<void*, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_PtrUnorderedMapBit;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<int64_t, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_Int64UnorderedMapBit;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<std::string, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_StringUnorderedMapBit;
        }
        else
        {
            static_assert(ConstexprFalse<T> && "UNSUPPORTED TYPE");
        }
        using AddPointerType = std::add_pointer_t<T>;
        std::type_index add_pointer_type_index = typeid(AddPointerType);
        pointer_type_point_to_type_index_map_ref.insert(std::make_pair(add_pointer_type_index, return_type));
        auto uninitialize_pointer_type_map_iterator = uninitialize_pointer_type_map_ref.find(add_pointer_type_index);
        if (uninitialize_pointer_type_map_iterator != uninitialize_pointer_type_map_ref.end())
        {
            uninitialize_pointer_type_map_iterator->second->_PointToType = return_type;
            uninitialize_pointer_type_map_ref.erase(uninitialize_pointer_type_map_iterator);
        }
        type_index_map_ref[type_index] = return_type;
        return return_type;
    }

    MYGUI_API bool VerifyStaticTypeInitializationResult();

}









