#include "udpflow/CommandHandler.h"
#include "udpflow/CommandProcessor.h"
#include "udpflow/Receiver.h"
#include "udpflow/Server.h"
#include "udpflow/Stat.h"
#include "udpflow/StatCollector.h"
#include "udpflow/Utils.h"

#include <codecvt>
#include <functional>
#include <iostream>
#include <jni.h>
#include <locale>
#include <string>

namespace
{

std::function<void(const std::string &)> g_output;
std::function<void()> g_start_timer;
std::function<void()> g_stop_timer;
std::unique_ptr<udpflow::Server> g_server;

std::shared_ptr<udpflow::Stat> getStat()
{
	static const std::shared_ptr<udpflow::Stat> g_stat = std::make_shared<udpflow::Stat>();
	return g_stat;
}

class JniCommandHandler : public udpflow::CommandHandler
{
	using udpflow::CommandHandler::CommandHandler;

public:
	static std::shared_ptr<JniCommandHandler> GetInstance()
	{
		static auto instance = std::make_shared<JniCommandHandler>(getStat());
		return instance;
	}

public:
	void StartMeasure() override
	{
		if (g_start_timer)
			g_start_timer();
	}
	void StopMeasure() override
	{
		if (g_stop_timer)
			g_stop_timer();
	}
};

} // namespace

void output(const std::string & str)
{
	if (!g_output)
		return;
	g_output(str);
}

extern "C" JNIEXPORT jobject JNICALL Java_com_example_udpflow_MainActivity_onTick(JNIEnv * env, jobject main_activity)
{
	JniCommandHandler::GetInstance()->onTick();
	return nullptr;
}
void createServer()
{
	if (!g_server)
	{
		const auto port = udpflow::Receiver::default_port;
		auto command_processor = std::make_shared<udpflow::CommandProcessor>(JniCommandHandler::GetInstance());
		g_server = std::make_unique<udpflow::Server>(std::move(command_processor), getStat());
		output(std::string("Start UDP Server on ") + std::to_string(port));
	}
}

extern "C" JNIEXPORT jobject JNICALL Java_com_example_udpflow_MainActivity_Init(JNIEnv * env, jobject main_activity)
{
	JavaVM * jvm = nullptr;
	env->GetJavaVM(&jvm);
	g_output = [jvm, main_activity = env->NewGlobalRef(main_activity)](const std::string & str)
	{
		JNIEnv * env = nullptr;
		jvm->AttachCurrentThread(&env, 0);
		if (!env)
			return;
		jclass cls = env->GetObjectClass(main_activity);
		jmethodID method = env->GetMethodID(cls, "appendOutput", "(Ljava/lang/String;)V");
		env->CallVoidMethod(main_activity, method, env->NewStringUTF(str.c_str()));
	};

	g_start_timer = [jvm, main_activity = env->NewGlobalRef(main_activity)]
	{
		JNIEnv * env = nullptr;
		jvm->AttachCurrentThread(&env, 0);
		if (!env)
			return;
		jclass cls = env->GetObjectClass(main_activity);
		jmethodID method = env->GetMethodID(cls, "onStartTimer", "()V");
		env->CallVoidMethod(main_activity, method);
	};
	g_stop_timer = [jvm, main_activity = env->NewGlobalRef(main_activity)]
	{
		JNIEnv * env = nullptr;
		jvm->AttachCurrentThread(&env, 0);
		if (!env)
			return;
		jclass cls = env->GetObjectClass(main_activity);
		jmethodID method = env->GetMethodID(cls, "onStopTimer", "()V");
		env->CallVoidMethod(main_activity, method);
	};

	udpflow::OutputIPv4Addresses();

	createServer();

	JniCommandHandler::GetInstance()->StartStat();
	JniCommandHandler::GetInstance()->StartMeasure();

	return nullptr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_udpflow_MainActivity_UdpServerIsActive(JNIEnv * env, jobject /* this */)
{
	return !!g_server;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_udpflow_MainActivity_UdpServerTurnOn(JNIEnv * env, jobject /* this */)
{
	createServer();

	if (g_start_timer)
		g_start_timer();
	return nullptr;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_udpflow_MainActivity_UdpServerTurnOff(JNIEnv * env, jobject /* this */)
{
	if (g_server)
	{
		output("Stop UDP Server");
		g_server.reset();
	}
	if (g_stop_timer)
		g_stop_timer();
	return nullptr;
}

extern "C" JNIEXPORT jstring JNICALL Java_com_example_udpflow_MainActivity_GetStat(JNIEnv * env, jobject /* this */)
{
	if (g_server)
	{
		return env->NewStringUTF(getStat()->GetStat().c_str());
	}
	return nullptr;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_udpflow_MainActivity_GetStatCSVDump(JNIEnv * env, jobject /* this */)
{
	return env->NewStringUTF(JniCommandHandler::GetInstance()->GetStatDump().c_str());
}

extern "C" JNIEXPORT jobject JNICALL Java_com_example_udpflow_MainActivity_SendCommandTo(JNIEnv * env, jobject /* this */, jstring ip, jstring cmd)
{
	const jsize ip_str_len = env->GetStringUTFLength(ip);
	const char * ip_str = env->GetStringUTFChars(ip, (jboolean *)0);

	const jsize cmd_str_len = env->GetStringUTFLength(cmd);
	const char * cmd_str = env->GetStringUTFChars(cmd, (jboolean *)0);

	udpflow::Sender::Send(std::string(cmd_str, cmd_str_len), std::string(ip_str, ip_str_len));
	env->ReleaseStringUTFChars(ip, ip_str);
	env->ReleaseStringUTFChars(cmd, cmd_str);
	return nullptr;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_udpflow_MainActivity_StartSend(JNIEnv * env, jobject /* this */, jstring ip)
{
	const jsize ip_str_len = env->GetStringUTFLength(ip);
	const char * ip_str = env->GetStringUTFChars(ip, (jboolean *)0);

	createServer();

	JniCommandHandler::GetInstance()->StartTransferTo(
		udpflow::IpFromString(std::string(ip_str, ip_str_len)),
		udpflow::constants::default_receive_port);
	env->ReleaseStringUTFChars(ip, ip_str);
	return nullptr;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_udpflow_MainActivity_StopSend(JNIEnv * env, jobject /* this */)
{
	JniCommandHandler::GetInstance()->StopTransfer();
	return nullptr;
}
