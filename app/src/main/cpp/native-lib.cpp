#include "udpflow/CommandHandler.h"
#include "udpflow/Server.h"
#include "udpflow/Stat.h"
#include "udpflow/StatCollector.h"

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
	void onTick()
	{
		if (stat_collector_)
			stat_collector_->onTick();
	}

	std::string GetStatDump() const
	{
		if (!stat_collector_)
			return {};

		return stat_collector_->dump();
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

	void StartStat() override
	{
		stat_collector_ = std::make_unique<udpflow::StatCollector>(getStat());
	}

	void StopStat() override
	{
		stat_collector_.reset();
	}

private:
	std::unique_ptr<udpflow::StatCollector> stat_collector_;
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

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_udpflow_MainActivity_SetOutput(JNIEnv * env, jobject main_activity)
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
	if (!g_server)
	{
		const auto port = udpflow::Server::default_port;
		g_server = std::make_unique<udpflow::Server>(JniCommandHandler::GetInstance(), getStat(), port);
		output(std::string("Start UDP Server on ") + std::to_string(port));
	}

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

extern "C" JNIEXPORT jstring JNICALL Java_com_example_udpflow_MainActivity_GetStatCSVDump(JNIEnv * env, jobject /* this */)
{
	return env->NewStringUTF(JniCommandHandler::GetInstance()->GetStatDump().c_str());
}
