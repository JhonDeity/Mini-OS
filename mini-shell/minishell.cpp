#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>

#define PROMPT "vboxshell> " 


std::vector<std::string> parse_line(const std::string& line, std::string& output_file);
void execute_command(const std::vector<std::string>& args, const std::string& output_file);


int main() {
    std::string line;
    
    while (true) {
        
        std::cout << PROMPT;
        if (!std::getline(std::cin, line)) {
            break; 
        }
        
        
        if (line == "salir") {
            break;
        }

        
        std::string output_file = ""; 

        
        std::vector<std::string> args = parse_line(line, output_file);
        
        if (!args.empty()) {
            execute_command(args, output_file);
        }
    }
    
    return 0;
}


std::vector<std::string> parse_line(const std::string& line, std::string& output_file) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    bool found_redirect = false;

    while (ss >> token) {
        if (token == ">") {
            found_redirect = true;
            continue; 
        }
        
        if (found_redirect) {
            
            output_file = token;
            found_redirect = false; 
        } else {
            tokens.push_back(token);
        }
    }
    
    
    if (found_redirect) { 
        std::cerr << "shell: Error de sintaxis, falta archivo para redirección.\n";
        tokens.clear(); 
    }
    
    return tokens;
}



void execute_command(const std::vector<std::string>& args, const std::string& output_file) {
    pid_t pid;
    int status;
    
    
    pid = fork();

    if (pid < 0) {
        
        perror("shell: Error en fork");
        return;
    } 
    else if (pid == 0) {
        

        
        if (!output_file.empty()) {
            int fd_out = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            
            if (fd_out == -1) {
               
                std::cerr << "shell: No se pudo abrir el archivo de salida: " << output_file << "\n";
                perror("open error");
                exit(EXIT_FAILURE);
            }
            
            
            if (dup2(fd_out, STDOUT_FILENO) == -1) {
                
                perror("shell: Error en dup2 para redirección");
                exit(EXIT_FAILURE);
            }
            close(fd_out); 
        }

        
        std::vector<char*> c_args;
        for (const auto& arg : args) {
            
            
            c_args.push_back(const_cast<char*>(arg.c_str())); 
        }
        c_args.push_back(nullptr); 

        
        

        
        if (args[0].find('/') == std::string::npos) {
            
            std::string bin_path = "/bin/" + args[0];
            execv(bin_path.c_str(), c_args.data()); 
            
        }
        
        
        
        execvp(c_args[0], c_args.data()); 
        
        
        
        
        fprintf(stderr, "shell: No se pudo ejecutar el comando: %s\n", args[0].c_str());
        perror("execvp error");
        exit(EXIT_FAILURE); 
    } 
    else {
        
        
        
        if (waitpid(pid, &status, 0) == -1) {
            
            perror("shell: Error en waitpid");
        }
    }
}