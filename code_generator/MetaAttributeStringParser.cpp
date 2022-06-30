#include "MetaAttributeStringParser.h"
#include <iostream>

bool IsAlphaOrUnderscore(char character)
{
	return std::isalpha(character) || character == '_';
}

bool IsAttributeNameCharacter(char character)
{
	return std::isdigit(character) || std::isalpha(character) || character == '_';
}

bool ParseAttributeName(const std::string& attribute_string_ref, size_t& index_ref, std::string& attribute_name, std::string& attribute_value)
{
	//( a = b )
	// ^
	while (attribute_string_ref.size() > index_ref && std::isspace(attribute_string_ref[index_ref]))
	{
		index_ref++;
	}
	//( a = b )
	//  ^
	if (attribute_string_ref.size() > index_ref && 
		IsAlphaOrUnderscore(attribute_string_ref[index_ref]))
	{
		attribute_name.push_back(attribute_string_ref[index_ref]);
		index_ref++;
		while (attribute_string_ref.size() > index_ref &&
			(std::isdigit(attribute_string_ref[index_ref]) || IsAlphaOrUnderscore(attribute_string_ref[index_ref])))
		{
			attribute_name.push_back(attribute_string_ref[index_ref]);
			index_ref++;
		}
		//( a = b )
		//   ^
		while (attribute_string_ref.size() > index_ref && 
			std::isspace(attribute_string_ref[index_ref]))
		{
			index_ref++;
		}
		if (attribute_string_ref.size() > index_ref)
		{
			//( a = b )
			//    ^
			if (attribute_string_ref[index_ref] == '=')
			{
				index_ref++;
				//( a = b )
				//     ^
				while (attribute_string_ref.size() > index_ref && 
					std::isspace(attribute_string_ref[index_ref]))
				{
					index_ref++;
				}
				if (attribute_string_ref.size() > index_ref)
				{
					char current_char = attribute_string_ref[index_ref];
					//( a = "b" )
					//      ^
					if (current_char == '"')
					{
						attribute_value.push_back(attribute_string_ref[index_ref]);
						index_ref++;
						current_char = attribute_string_ref[index_ref];
						//( a = "b" )
						//       ^
						while (attribute_string_ref.size() > index_ref)
						{
							current_char = attribute_string_ref[index_ref];
							//( a = "b" )
							//        ^
							if (current_char == '"')
							{
								attribute_value.push_back(attribute_string_ref[index_ref]);
								index_ref++;
								break;
							}
							else if (current_char == '\\')
							{
								if (attribute_string_ref.size() > index_ref + 1)
								{
									char next_char = attribute_string_ref[index_ref + 1];
									if (next_char == '"')
									{
										attribute_value.push_back('\\');
										attribute_value.push_back('"');
										index_ref++;
										index_ref++;
									}
									else if (next_char == '\\')
									{
										attribute_value.push_back('\\');
										attribute_value.push_back('\\');
										index_ref++;
										index_ref++;
									}
									else
									{
										std::cout << "parse incorrect format meta attribute string [" << attribute_string_ref << "] at index " << index_ref << std::endl;
										return false;
									}
								}
								else
								{
									std::cout << "parse incorrect format meta attribute string [" << attribute_string_ref << "] at index " << index_ref << std::endl;
									return false;
								}
							}
							else
							{
								attribute_value.push_back(attribute_string_ref[index_ref]);
								index_ref++;
							}
						}
					}
					//( a = 012ULL )
					//      ^   
					else if (std::isdigit(current_char))
					{
						//( a = 012ULL )
						//      ^^^^^^			
						while (attribute_string_ref.size() > index_ref)
						{
							if (std::isspace(attribute_string_ref[index_ref]))
							{
								break;
							}
							attribute_value.push_back(attribute_string_ref[index_ref]);
							index_ref++;
						}
					}
					else
					{
						attribute_value.push_back('"');
						//( a = bcdef )
						//      ^^^^^		
						while (attribute_string_ref.size() > index_ref)
						{
							if (IsAttributeNameCharacter(attribute_string_ref[index_ref]))
							{
								attribute_value.push_back(attribute_string_ref[index_ref]);
								index_ref++;
							}
							else
							{
								break;
							}
						}
						attribute_value.push_back('"');
					}
					//( a = b )
					//       ^
					while (attribute_string_ref.size() > index_ref &&
						std::isspace(attribute_string_ref[index_ref]))
					{
						index_ref++;
					}
					//( a = b ,)
					//        ^
					if (attribute_string_ref.size() > index_ref)
					{
						if (attribute_string_ref[index_ref] == ',')
						{
							index_ref++;
						}
						else
						{
							std::cout << "parse incorrect format meta attribute string [" << attribute_string_ref << "] at index " << index_ref << std::endl;
						}
					}
				}
			}
			else if (attribute_string_ref[index_ref] == ',')
			{
				index_ref++;
			}
			else
			{
				std::cout << "parse incorrect format meta attribute string [" << attribute_string_ref << "] at index " << index_ref << std::endl;
				return false;
			}
		}
	}
	else
	{
		std::cout << "parse incorrect format meta attribute string [" << attribute_string_ref << "] at index " << index_ref << std::endl;
		return false;
	}
	return true;
}


std::map<std::string, std::string> ParseMetaAttributeString(std::string& meta_attribute_string_ref)
{
	std::map<std::string, std::string> attributes;
	size_t string_search_index = 0;
	while (meta_attribute_string_ref.size() > string_search_index)
	{
		std::string attribute_name;
		std::string attribute_value;
		if (!ParseAttributeName(meta_attribute_string_ref, string_search_index, attribute_name, attribute_value))
		{
			break;
		}
		if (!attribute_name.empty())
		{
			attributes.insert_or_assign(attribute_name, attribute_value);
		}
	}
	return attributes;
}
//bool FindReflectAnnotation(const clang::Decl* CheckedDecl, std::map<std::string, std::string>& OutMetadata) {
//    OutMetadata.clear();
//    if (CheckedDecl->hasAttrs()) {
//        for (auto Attr : CheckedDecl->getAttrs()) {
//
//            if (Attr->getKind() == clang::attr::Annotate)
//            {
//                std::string RowStringBuffer;
//                llvm::raw_string_ostream RowStringOutputStream(RowStringBuffer);
//                Attr->printPretty(RowStringOutputStream, clang::PrintingPolicy(clang::LangOptions()));
//                std::string AttrFullString(RowStringOutputStream.str());
//                constexpr static size_t ReflAnnotateStartSize = sizeof("[[clang::annotate(\"Refl") - 1;
//                constexpr static size_t ReflAnnotateEndSize = sizeof("\")]]") - 1;
//                size_t ReflAnnotateStartPos = AttrFullString.find("[[clang::annotate(\"Refl");
//                size_t ReflAnnotateEndPos = AttrFullString.rfind("\")]]");
//                if (ReflAnnotateStartPos != std::string::npos &&
//                    ReflAnnotateEndPos != std::string::npos &&
//                    ReflAnnotateStartPos <= 1 &&
//                    (ReflAnnotateEndPos == (AttrFullString.size() - 4)))
//                {
//                    std::string MetaString;
//                    MetaString.reserve(AttrFullString.size() - ReflAnnotateStartSize - ReflAnnotateEndSize - ReflAnnotateStartPos);
//                    std::string MetadataKey, MetadataValue;
//                    size_t OffsetIndex = ReflAnnotateStartSize + ReflAnnotateStartPos;
//                    size_t LastIndex = AttrFullString.size() - ReflAnnotateEndSize;
//                    if (AttrFullString[OffsetIndex] == ',') OffsetIndex++;
//                    while (OffsetIndex < LastIndex)
//                    {
//                        while (OffsetIndex < LastIndex && std::isspace(AttrFullString[OffsetIndex])) OffsetIndex++;
//                        while (OffsetIndex < LastIndex && (
//                            std::isalpha(AttrFullString[OffsetIndex]) ||
//                            std::isdigit(AttrFullString[OffsetIndex]) ||
//                            AttrFullString[OffsetIndex] == '_'
//                            ))
//                        {
//                            MetadataKey.push_back(AttrFullString[OffsetIndex]);
//                            OffsetIndex++;
//                        }
//                        while (OffsetIndex < LastIndex && std::isspace(AttrFullString[OffsetIndex])) OffsetIndex++;
//                        if (AttrFullString[OffsetIndex] == '=')
//                        {
//                            OffsetIndex++;
//                            while (OffsetIndex < LastIndex && std::isspace(AttrFullString[OffsetIndex])) OffsetIndex++;
//                            if (AttrFullString[OffsetIndex] == '\"')
//                            {
//                                OffsetIndex++;
//                                while (OffsetIndex < LastIndex && AttrFullString[OffsetIndex] != '\"')
//                                {
//                                    if (AttrFullString[OffsetIndex] == '\\')
//                                    {
//                                        if (AttrFullString[OffsetIndex] == '\\' || AttrFullString[OffsetIndex] == '\"')
//                                        {
//                                            MetadataValue.push_back(AttrFullString[OffsetIndex]);
//                                            OffsetIndex++;
//                                        }
//                                        else
//                                        {
//                                            throw std::runtime_error(std::format("{:s} illegal metadata macro, correct format should be RMETADATA(A=B, C = \"D\")", Attr->getLocation().printToString(*SourceManager)));
//                                        }
//                                    }
//                                    MetadataValue.push_back(AttrFullString[OffsetIndex]);
//                                    OffsetIndex++;
//                                }
//                                OffsetIndex++;
//                            }
//                            else
//                            {
//                                if (std::isdigit(AttrFullString[OffsetIndex]))
//                                {
//                                    while (OffsetIndex < LastIndex && (
//                                        std::isdigit(AttrFullString[OffsetIndex]) ||
//                                        AttrFullString[OffsetIndex] != '.'
//                                        ))
//                                    {
//                                        MetadataValue.push_back(AttrFullString[OffsetIndex]);
//                                        OffsetIndex++;
//                                    }
//                                }
//                                else
//                                {
//                                    while (OffsetIndex < LastIndex && (
//                                        std::isalpha(AttrFullString[OffsetIndex]) ||
//                                        std::isdigit(AttrFullString[OffsetIndex]) ||
//                                        AttrFullString[OffsetIndex] == '_'
//                                        ))
//                                    {
//                                        MetadataValue.push_back(AttrFullString[OffsetIndex]);
//                                        OffsetIndex++;
//                                    }
//                                }
//                            }
//                            while (OffsetIndex < LastIndex && std::isspace(AttrFullString[OffsetIndex])) OffsetIndex++;
//                            if (OutMetadata.contains(MetadataKey))
//                            {
//                                throw std::runtime_error(std::format("{:s} illegal metadata macro, correct format should be RMETADATA(A=B, C = \"D\")", Attr->getLocation().printToString(*SourceManager)));
//                            }
//                            OutMetadata.insert(std::make_pair(MetadataKey, MetadataValue));
//                            if (AttrFullString[OffsetIndex] == ',')
//                            {
//                                OffsetIndex++;
//                            }
//                            else if (OffsetIndex == LastIndex)
//                            {
//
//                            }
//                            else
//                            {
//                                throw std::runtime_error(std::format("{:s} illegal metadata macro, correct format should be RMETADATA(A=B, C = \"D\")", Attr->getLocation().printToString(*SourceManager)));
//                            }
//                        }
//                        else if (AttrFullString[OffsetIndex] == ',')
//                        {
//                            if (OutMetadata.contains(MetadataKey))
//                            {
//                                throw std::runtime_error(std::format("{:s} illegal metadata macro, correct format should be RMETADATA(A=B, C = \"D\")", Attr->getLocation().printToString(*SourceManager)));
//                            }
//                            OutMetadata.insert(std::make_pair(MetadataKey, MetadataValue));
//                            OffsetIndex++;
//                        }
//                        else if (OffsetIndex == LastIndex)
//                        {
//                            if (OutMetadata.contains(MetadataKey))
//                            {
//                                throw std::runtime_error(std::format("{:s} illegal metadata macro, correct format should be RMETADATA(A=B, C = \"D\")", Attr->getLocation().printToString(*SourceManager)));
//                            }
//                            OutMetadata.insert(std::make_pair(MetadataKey, MetadataValue));
//                        }
//                        else
//                        {
//                            throw std::runtime_error(std::format("{:s} illegal metadata macro, correct format should be RMETADATA(A=B, C = \"D\")", Attr->getLocation().printToString(*SourceManager)));
//                        }
//                        MetadataKey.clear();
//                        MetadataValue.clear();
//                    }
//                    return true;
//                }
//            }
//        }
//    }
//    return false;
//}
