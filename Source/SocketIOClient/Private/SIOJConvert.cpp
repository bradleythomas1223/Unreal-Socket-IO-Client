// Fill out your copyright notice in the Description page of Project Settings.

#include "SocketIOClientPrivatePCH.h"
#include "SIOJConvert.h"

typedef TJsonWriterFactory< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriterFactory;
typedef TJsonWriter< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriter;

TSharedPtr<FJsonValue> USIOJConvert::ToJsonValue(const sio::message::ptr& Message)
{
	auto flag = Message->get_flag();

	if (flag == sio::message::flag_integer)
	{
		return MakeShareable(new FJsonValueNumber(Message->get_int()));
	}
	else if (flag == sio::message::flag_double)
	{
		return MakeShareable(new FJsonValueNumber(Message->get_double()));
	}
	else if (flag == sio::message::flag_string)
	{
		return MakeShareable(new FJsonValueString(FStringFromStd(Message->get_string())));
	}
	else if (flag == sio::message::flag_binary)
	{
		//Todo: add support for this somehow?
		return MakeShareable(new FJsonValueString(FString("<binary not supported in FJsonValue, use raw sio::message methods>")));
	}
	else if (flag == sio::message::flag_array)
	{
		auto MessageVector = Message->get_vector();
		TArray< TSharedPtr<FJsonValue> > InArray;

		InArray.Reset(MessageVector.size());

		for (auto ItemMessage : MessageVector)
		{
			InArray.Add(ToJsonValue(ItemMessage));
		}
		
		return MakeShareable(new FJsonValueArray(InArray));
	}
	else if (flag == sio::message::flag_object)
	{
		auto  MessageMap = Message->get_map();
		TSharedPtr<FJsonObject> InObject = MakeShareable(new FJsonObject());

		for (auto MapPair : MessageMap)
		{
			InObject->SetField(FStringFromStd(MapPair.first), ToJsonValue(MapPair.second));
		}

		return MakeShareable(new FJsonValueObject(InObject));
	}
	else if (flag == sio::message::flag_boolean)
	{
		bool InBoolean = false;

		return MakeShareable(new FJsonValueBoolean(InBoolean));
	}
	else if (flag == sio::message::flag_null)
	{
		return MakeShareable(new FJsonValueNull());
	}
	else 
	{
		return MakeShareable(new FJsonValueNull());
	}
}



sio::message::ptr USIOJConvert::ToSIOMessage(const TSharedPtr<FJsonValue>& JsonValue)
{
	if (JsonValue->Type == EJson::None)
	{
		return sio::null_message::create();
	}
	else if (JsonValue->Type == EJson::Null)
	{
		return sio::null_message::create();
	}
	else if (JsonValue->Type == EJson::String)
	{
		return sio::string_message::create(StdString(JsonValue->AsString()));
	}
	else if (JsonValue->Type == EJson::Number)
	{
		return sio::double_message::create(JsonValue->AsNumber());
	}
	else if (JsonValue->Type == EJson::Boolean)
	{
		return sio::bool_message::create(JsonValue->AsBool());
	}
	else if (JsonValue->Type == EJson::Array)
	{
		auto ValueArray = JsonValue->AsArray();
		auto ArrayMessage = sio::array_message::create();

		for (auto ItemValue : ValueArray)
		{
			//must use get_vector() for each
			ArrayMessage->get_vector().push_back(ToSIOMessage(ItemValue));
		}

		return ArrayMessage;
	}
	else if (JsonValue->Type == EJson::Object)
	{
		auto ValueTmap = JsonValue->AsObject()->Values;

		auto ObjectMessage = sio::object_message::create();

		for (auto ItemPair : ValueTmap)
		{
			//important to use get_map() directly to insert the key in the correct map and not a pointer copy
			ObjectMessage->get_map()[StdString(ItemPair.Key)] = ToSIOMessage(ItemPair.Value);
		}

		return ObjectMessage;
	}
	else
	{
		return sio::null_message::create();
	}
}

std::string USIOJConvert::StdString(FString UEString)
{
	return std::string(TCHAR_TO_UTF8(*UEString));	//TCHAR_TO_ANSI try this string instead?
}

FString USIOJConvert::FStringFromStd(std::string StdString)
{
	return FString(StdString.c_str());
}

FString USIOJConvert::ToJsonString(const TSharedPtr<FJsonObject>& JsonObject)
{
	FString OutputString;
	TSharedRef< FCondensedJsonStringWriter > Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	return OutputString;
}

FString USIOJConvert::ToJsonString(const TArray<TSharedPtr<FJsonValue>>& JsonValueArray)
{
	FString OutputString;
	TSharedRef< FCondensedJsonStringWriter > Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
	FJsonSerializer::Serialize(JsonValueArray, Writer);
	return OutputString;
}

FString USIOJConvert::ToJsonString(const TSharedPtr<FJsonValue>& JsonValue)
{
	FString OutputString;
	TSharedRef< FCondensedJsonStringWriter > Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
	FJsonSerializer::Serialize(JsonValue,FString(TEXT("value")), Writer);
	return OutputString;
}

TSharedPtr<FJsonObject> USIOJConvert::ToJsonObject(const FString& JsonString)
{
	TSharedPtr< FJsonObject > JsonObject = MakeShareable(new FJsonObject);
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(*JsonString);
	FJsonSerializer::Deserialize(Reader, JsonObject);
	return JsonObject;
}