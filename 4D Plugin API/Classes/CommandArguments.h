#ifndef COMMAND_ARGUMENTS_H
#define COMMAND_ARGUMENTS_H

#include "4DPluginAPI.h"
#include <stdexcept>

template<size_t Count>
class CommandArguments
{
public:
	PA_Variable values[Count];

	CommandArguments()
	{
		for(size_t i = 0; i < Count; ++i)
			values[i] = PA_CreateVariable(eVK_Undefined);
	}

	~CommandArguments()
	{
		for(size_t i = 0; i < Count; ++i)
			PA_ClearVariable(&values[i]);
	}

	void executeChecked(PA_long32 command)
	{
		PA_Variable returned = PA_ExecuteCommandByID(command, values, Count);
		// Failure outputs are unspecified, including the raw-kind-zero handle failure.
		if(PA_GetLastError() != eER_NoErr)
			throw std::runtime_error("4D picture command failed");
		PA_ClearVariable(&returned);
	}

	CommandArguments(const CommandArguments&) = delete;
	CommandArguments& operator=(const CommandArguments&) = delete;
};

#endif
