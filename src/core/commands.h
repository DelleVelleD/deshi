#pragma once
#ifndef DESHI_COMMANDS_H
#define DESHI_COMMANDS_H

#include "../utils/defines.h"
#include "../utils/any.h"
#include "../core.h"

#include <string>
#include <functional>
#include <vector>

struct CommandInfo{
	std::string name;
	std::string description;
	u32 minArgs, maxArgs;
	std::function<void(std::vector<std::string>)> proc;
};

std::vector<CommandInfo> commands;

void
RunCommand(CommandInfo command, std::vector<std::string> args){
	if(command.minArgs != -1){
		if(args.size() < command.minArgs){
			if(command.minArgs == command.maxArgs){
				ERROR("Command '", command.name, "' requires exactly ", command.minArgs, " arguments"); return;
			}else{
				ERROR("Command '", command.name, "' requires at least ", command.minArgs, " arguments"); return;
			}
		}else if(args.size() > command.maxArgs){
			if(command.minArgs == command.maxArgs){
				ERROR("Command '", command.name, "' requires exactly ", command.maxArgs, " arguments"); return;
			}else{
				ERROR("Command '", command.name, "' requires at most ", command.maxArgs, " arguments"); return;
			}
		}
	}
	
	command.proc(args);
}

void
RunCommand(std::string line){
	std::string command = "";
	std::vector<std::string> args;
	
	size_t pos = 0, old_pos;
	while(pos != -1){
		old_pos = pos;
		pos = line.find_first_of(' ', pos);
		if(old_pos == 0){
			command = line.substr(old_pos, pos - old_pos);
		}else{
			args.push_back(line.substr(old_pos, pos - old_pos));
		}
		if(pos != -1) pos++;
	}
	
	if(command == "") return;
	
	for(auto it : commands){
		if(command == it.name){
			if(it.proc){
				return RunCommand(it, args);
			}else{
				ERROR("Command '", it.name, "' has no procedure");
			}
		}
	}
	
	ERROR("Unknown command '", command, "'");
}

static_internal void
AddCommand(std::function<void(std::vector<std::string>)> proc, std::string name, std::string description = "", u32 minArgs = -1, u32 maxArgs = -1){
	if(maxArgs == -1) { maxArgs = minArgs; } //if only max args not specified, it uses exactly that min args
	CommandInfo info;
	info.name = name;
	info.description = description;
	info.minArgs = minArgs;
	info.maxArgs = maxArgs;
	info.proc = proc;
	commands.push_back(info);
}

//////////////////
//// commands ////
//////////////////

static_internal void
command_quit(std::vector<std::string>){
	PRINT("I QUit!");
}

static_internal void
command_add(std::vector<std::string>){
	
}

///////////////////////
//// commands init ////
///////////////////////

void
InitCommands(){
	AddCommand(command_quit, "quit", "Exits the application");
	AddCommand(command_add, "add");
}

#endif //DESHI_COMMANDS_H
