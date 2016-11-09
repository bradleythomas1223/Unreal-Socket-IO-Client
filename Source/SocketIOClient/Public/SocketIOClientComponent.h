#pragma once

#include "sio_client.h"
#include "Components/ActorComponent.h"
#include "SocketIOClientComponent.generated.h"

UENUM(BlueprintType)
enum EMessageTypeFlag
{
	FLAG_INTEGER,
	FLAG_DOUBLE,
	FLAG_STRING,
	FLAG_BINARY,
	FLAG_ARRAY,
	FLAG_OBJECT,
	FLAG_BOOLEAN,
	FLAG_NULL
};

//TODO: convert sio::message to UE struct for more flexible use
USTRUCT()
struct FSIOMessage
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "SocketIO Message Properties")
	TEnumAsByte<EMessageTypeFlag> MessageFlag;

	//Internal UE storage
	FJsonObject Object;
};

UENUM(BlueprintType)
enum EConnectionCloseReason
{
	CLOSE_REASON_NORMAL,
	CLOSE_REASON_DROP
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSIOCEventSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSIOCSocketEventSignature, FString, Namespace);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSIOCOpenEventSignature, FString, SessionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSIOCCloseEventSignature, TEnumAsByte<EConnectionCloseReason>, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSIOCNameDataEventSignature, FString, Name, FString, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSIOCEventJsonSignature, FString, Name, FString, Data);

//todo swap with fjsonvalue

UCLASS(ClassGroup = "Networking", meta = (BlueprintSpawnableComponent))
class SOCKETIOCLIENT_API USocketIOClientComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:

	//Async events
	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FSIOCOpenEventSignature OnConnected;

	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FSIOCCloseEventSignature OnDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FSIOCSocketEventSignature OnSocketNamespaceConnected;

	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FSIOCSocketEventSignature OnSocketNamespaceDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FSIOCEventSignature OnFail;

	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FSIOCNameDataEventSignature On;

	//Default properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = defaults)
	FString AddressAndPort;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = defaults)
	bool ShouldAutoConnect;

	UPROPERTY(BlueprintReadWrite, Category = "SocketIO Properties")
	FString SessionId;

	/**
	* Connect to a socket.io server, optional if auto-connect is set
	*
	* @param AddressAndPort	the address in URL format with port
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketIO Functions")
	void Connect(FString InAddressAndPort);

	UFUNCTION(BlueprintCallable, Category = "SocketIO Functions")
	void Disconnect();

	/**
	* Emit a string event with a string action
	*
	* @param Name	Event name
	* @param Data Data string
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketIO Functions")
	void EmitString(FString Name, FString Data = FString(TEXT("")), FString Namespace = FString(TEXT("/")));

	UFUNCTION(BlueprintCallable, Category = "SocketIO Functions")
	void Emit(FString Name, UVaRestJsonValue* Data, FString Namespace = FString(TEXT("/")));

	//Binary data version, only available in C++
	void EmitBuffer(FString Name, uint8* Data, int32 DataLength, FString Namespace = FString(TEXT("/")));

	//Raw sio::message emit, only available in C++
	void EmitRaw(FString Name, const sio::message::list& MessageList = nullptr, FString Namespace = FString(TEXT("/")));
	void EmitRawWithCallback(FString Name, const sio::message::list& MessageList = nullptr, TFunction<void(const sio::message::list&)> ResponseFunction = nullptr, FString Namespace = FString(TEXT("/")));
	


	/**
	* Emit a message
	*
	* @param Event		Event name
	* @param TFunction	Lambda callback, raw flavor
	* @param Namespace	Optional namespace, defaults to default namespace
	*/


	void OnRawEvent(FString Event, TFunction< void(const FString&, const sio::message::ptr&)> CallbackFunction, FString Namespace = FString(TEXT("/")));
	void OnEvent(FString Event, TFunction< void(const FString&, const FJsonValue&)> CallbackFunction, FString Namespace = FString(TEXT("/")));

	//Blueprint
	//UFUNCTION(BlueprintCallable, Category = "SocketIO Functions")
	//void OnEvent(FString Event, TFunction< void()> InFunction);


	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

	//C++ version of binding arbitrary lambda functions to events, if you want only to be notified and don't care about arguments
	void BindLambdaToEvent(TFunction< void()> InFunction, FString Name, FString Namespace = FString(TEXT("/")));

	//When you care about the data you get
	void BindStringMessageLambdaToEvent(TFunction< void(const FString&, const FString&)> InFunction, FString Name, FString Namespace = FString(TEXT("/")));
	void BindBinaryMessageLambdaToEvent(TFunction< void(const FString&, const TArray<uint8>&)> InFunction, FString Name, FString Namespace = FString(TEXT("/")));

	//Raw sio::message lambda
	void BindRawMessageLambdaToEvent(TFunction< void(const FString&, const sio::message::ptr&)> InFunction, FString Name, FString Namespace = FString(TEXT("/")));

protected:
	sio::client PrivateClient;
};