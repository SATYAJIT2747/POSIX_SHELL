#include <iostream>
#include <string>
#include <termios.h> // for changing the terminal modes cannoniacal mode <----> non-canonical or raw mode
#include <vector>
#include <sys/wait.h> // for waitpid()
#include <cstring>
#include <unistd.h>      // for gethostname() , getcwd()
#include <sys/utsname.h> // for uname()
#include <pwd.h>         // for getuid(), getpwuid()
#include <limits.h>
#include <fcntl.h>    // need for open() file operation
#include <dirent.h>   // for directory operations like opendir() , readdir() , closedir() , closedir()
#include <sys/stat.h> // for stat()
#include <grp.h>      // for getgrgid() to get the grp id
#include <fstream>    // for history
#include <deque>      // for history handling
#include <algorithm>  // for sort function to be used during command completion after tab pressed
using namespace std;
string shell_home_dir, prev_dir;
deque<string> command_history;   // to store the history of commands
const int MAX_HISTORY_SIZE = 20; // maximum number of commands to store in history
string histroy_file_path;        // path to the history file
struct termios original_termios; // to store the original terminal satte
size_t history_index = 0;        // to navigate through the history by the up or down key
void displaytheprompt()
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == nullptr)
    {
        perror("getpwuid");
        return;
    }
    string username = pw->pw_name;
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        perror("gethostname");
        return;
    }
    string system_name = hostname;
    // get the current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        perror("getcwd");
        return;
    }
    string current_working_directory = cwd;

    // replace with ~ if cwd starts with  shell's home directory
    if (current_working_directory.find(shell_home_dir) == 0)
    {
        current_working_directory.replace(0, shell_home_dir.length(), "~");
    }
    // display the prompt
    cout << username << "@" << system_name << ":" << current_working_directory << "> ";
}

vector<string> get_command_completion(const string &prefix)
{
    vector<string> matchedcommands;

    // first we will search in the curren directory we are in i.e in (.)
    DIR *dir = opendir(".");
    if (dir != NULL)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            string entry_name = entry->d_name;
            if (entry_name.rfind(prefix, 0) == 0) // if the entry name starts with the prefix
            {
                struct stat path_stat;
                stat(entry_name.c_str(), &path_stat);
                if (S_ISDIR(path_stat.st_mode)) // check is directory
                {
                    matchedcommands.push_back(entry_name + "/"); // append / to directory name
                }
                else
                    matchedcommands.push_back(entry_name + " "); // its a file so add name and space so that u can type anothe argumnet
            }
        }
        closedir(dir);
    }

    // now we will search in the directories listed in the PATH environment variable i.e here we will fill  with commands
    string path_envirn = getenv("PATH");
    char *path_copy = new char[path_envirn.length() + 1];
    strcpy(path_copy, path_envirn.c_str());
    char *path_token = strtok(path_copy, ":");
    while (path_token != NULL)
    {

        DIR *path_dir = opendir(path_token);
        if (path_dir != NULL)
        {
            struct dirent *entry;
            while ((entry = readdir(path_dir)) != NULL)
            {
                string entry_name = entry->d_name;
                if (entry_name.rfind(prefix, 0) == 0) // if the entry name starts with the prefix i.e match found
                {
                    matchedcommands.push_back(entry_name + " "); // add space so that we can type next argument
                }
            }
            closedir(path_dir);
        }
        path_token = strtok(NULL, ":");
    }
    delete[] path_copy;
    // remove duplicates from matchedcommands
    sort(matchedcommands.begin(), matchedcommands.end());
    matchedcommands.erase(unique(matchedcommands.begin(), matchedcommands.end()), matchedcommands.end());
    return matchedcommands;
}
// load the history of commands from the history file
void load_command_history()
{
    histroy_file_path = shell_home_dir + "/.my_shell_history";
    ifstream history_file(histroy_file_path);
    if (history_file.is_open())
    {
        string line;
        while (getline(history_file, line))
        {
            command_history.push_back(line);
        }
        history_file.close();
    }
}

// to save history to the history file
void save_command_history()
{
    ofstream history_file(histroy_file_path, ios::out | ios::trunc);
    if (history_file.is_open())
    {
        for (const string &cmnd : command_history)
        {
            history_file << cmnd << endl;
        }
        history_file.close();
    }
}

// handle history  function will execute when the user types history.

void handle_history(const vector<char *> &args)
{
    int n_to_show = 10; // Default value is 10

    if (args.size() == 3)
    { // User provided a number:with history num then null: then print that much histrioy of commands
        try
        {
            n_to_show = stoi(args[1]);
        }
        catch (...)
        {
            cerr << "history: invalid argument" << endl;
            return;
        }
    }
    else if (args.size() > 3)
    { // handles "history" but with too many args
        cerr << "history: too many arguments" << endl;
        return;
    }

    int start_index = max(0, (int)(command_history.size()) - n_to_show);
    for (size_t i = start_index; i < command_history.size(); i++)
    {
        cout << " " << i + 1 << "\t" << command_history[i] << endl;
    }
}

// disable the raw mode and enable the canonical mode
void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// enable the raw mode to read character by character input without waiting for enter button pressed

// This is the corrected version of enablerawmode
// This is the fully corrected version of enablerawmode
// This is the fully corrected version of enablerawmode
void enablerawmode()
{
    // // Get the current terminal settings
    // tcgetattr(STDIN_FILENO, &original_termios);
    // // Set a function to run automatically on exit to restore the terminal
    // atexit(disableRawMode);

    struct termios raw = original_termios;
    // we will disable echo, canonical mode, (ISIG), and (IEXTEN)
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    //  we will disable Ctrl+S/Q and carriage return
    raw.c_iflag &= ~(IXON | ICRNL);

    // now disable all output processing  ike converting '\n' to '\r\n' or like that
    raw.c_oflag &= ~(OPOST);

    // we will tell read() to return immediately after 1 byte is received, without  timeout
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    // apply the new "raw" settings to the terminal
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

string takeinput() // for handling the arrow keys
{
    char c;
    string line = "";
    history_index = command_history.size(); // initially the history index is at the end of the deque which stores all command 20 last specifically
    while (read(STDIN_FILENO, &c, 1) == 1)  // read one char at a time until enter key is pressed
    {
        if (c == 4)
        { // code for Ctrl+D
            if (line.empty())
            { // Only exit if the line is empty
                cout << endl;
                exit(0); // Exit the shell
            }
        }
        if (c == '\n' || c == '\r') // Enter key pressed
        {
            cout << endl;
            break;
        }

        else if (c == 127) // backspace key is pressed
        {
            if (!line.empty())
            {
                line.pop_back();          // remove the last one
                cout << "\b \b" << flush; // and move the cursor back, print space to erase the character, and move back again
            }
        }
        else if (c == '\033') // escape sequence for arrow keys or \x1b then read the next 2 byte for [ and A or B
        {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) // read the next 1st byte
                continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) // read the next 2nd byte
                continue;

            if (seq[0] == '[')
            {
                if (seq[1] == 'A') // up arrow key
                {
                    if (history_index > 0)
                    {
                        history_index--;
                        line = command_history[history_index]; // move the history index  up means point to the last command
                        cout << "\r";                          // return carriage to the start of the line
                        displaytheprompt();
                        cout << line << "\033[K" << flush; // now to clear the line an
                    }
                }
                else if (seq[1] == 'B') // down arrow
                {
                    if (history_index < command_history.size())
                    {
                        history_index++;
                        line = (history_index == command_history.size()) ? "" : command_history[history_index]; // if we are at the end of the deque then line is empty else point to the next command
                        cout << "\r";                                                                           // return carriage to the start of the line
                        displaytheprompt();
                        cout << line << "\033[K" << flush; // now we will clear the line(for tht this escape sequence is printed) and print the new line
                    }
                }
            }
        }

        else if (c == '\t')
        {                                                   /// tab key is pressed for the auto completion of commands
            size_t last_space = line.find_last_of(" \t\n"); // go to the last space so that we can take the half typed word as a prefix
            string prefix = (last_space == string::npos) ? line : line.substr(last_space + 1);

            if (!prefix.empty())
            {
                vector<string> completions = get_command_completion(prefix);
                if (completions.size() == 1)
                { // single match found i.e directly complete it

                    string completion = completions[0];
                    string suffix = completion.substr(prefix.length());
                    line += suffix;
                    cout << suffix << flush; // force to print the completed part in the terminal after tab
                }
                else if (completions.size() > 1)
                {
                    // if mutiple matches found we will print them all in new lines
                    cout << endl; // NEW: Go to a new line to start the list
                    for (const string &match : completions)
                    {
                        // NEW: Print each match followed by a newline for a clean list
                        cout << match << "\t";
                    }

                    // NEW: Redraw the original prompt and what the user had typed
                    displaytheprompt();
                    cout << line << flush;
                }
            }
        }

        else // regular character input
        {
            line += c;
            cout << c << flush; // print  the character in the terminal
        }
    }
    return line;
}

// helper function for the recursive search command

bool recursive_search(const string &basedir, const string &filename)
{
    DIR *dir = opendir(basedir.c_str()); // open the directory
    if (dir == NULL)
        return false;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue; // skip the . and .. entries as these are self and parent dirs
        // check the entry matches the filename we are looking for
        if (strcmp(entry->d_name, filename.c_str()) == 0)
        {
            closedir(dir);
            return true; // found the file
        }

        // build the path :basedir:/entry name
        string path = basedir + "/" + entry->d_name;

        // check if this path a directory
        struct stat path_stat;
        stat(path.c_str(), &path_stat);
        if (S_ISDIR(path_stat.st_mode))
        {
            // recursively search in this directory to find the file
            if (recursive_search(path, filename))
            {
                closedir(dir);
                return true; // found the file in this subdirectory
            }
        }
    }
    closedir(dir);
    return false; // the file is not found in this directory or its subdirectories
}

// now to handle search
void handle_search(const vector<char *> &args)
{
    if (args.size() != 3)
    {
        cerr << "search: invalid number of arguments provided " << endl;
        return;
    }
    string filename = args[1]; // this is the file to search

    char cwd_buffer[PATH_MAX];
    getcwd(cwd_buffer, sizeof(cwd_buffer)); // current working directory

    if (recursive_search(string(cwd_buffer), filename))
    {
        cout << " True " << endl;
    }
    else
    {
        cout << "False" << endl;
    }
}

//=========== its a helper function to print the stats of a file=========

void printfilestats(const string &path, const string &name)
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) < 0)
    {
        perror(("stat :" + name).c_str());
        return;
    }
    // now we will print the file permissions
    string permiss = "";
    permiss += (S_ISDIR(file_stat.st_mode)) ? 'd' : '-';
    permiss += (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
    permiss += (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
    permiss += (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
    permiss += (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
    permiss += (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
    permiss += (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
    permiss += (file_stat.st_mode & S_IROTH) ? 'r' : '-';
    permiss += (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
    permiss += (file_stat.st_mode & S_IXOTH) ? 'x' : '-';
    cout << permiss << " ";

    // now to see the number of links
    cout << file_stat.st_nlink << " ";

    // now we will see the user and the grp name
    struct passwd *pw = getpwuid(file_stat.st_uid);
    struct group *gr = getgrgid(file_stat.st_gid);
    cout << (pw ? pw->pw_name : to_string(file_stat.st_uid)) << " ";
    cout << (gr ? gr->gr_name : to_string(file_stat.st_gid)) << " ";

    // now we will see the file size
    cout << file_stat.st_size << " ";

    // now we will see the last modified time of the file
    char timebuffer[80];
    strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M", localtime(&file_stat.st_mtime));
    cout << timebuffer << " ";

    // finally  we will print the file name

    cout << name << endl;
}

// ========== handler for the ls command  i.e to interpreat the ls command and its arguments=========

// ========== handler for the ls command  i.e to interpreat the ls command and its arguments=========
void handlelscomm(const vector<char *> &args)
{
    bool to_show_hidden = false;
    bool to_show_long_format = false;
    vector<string> directories; // to hold the directories whose contents we have to list

    // now we will parse the args to see if -a or -l is present or not
    for (size_t i = 1; i < args.size() - 1; i++)
    {
        if (args[i][0] == '-') // its  a flg to handle like a or l
        {
            for (size_t j = 1; j < strlen(args[i]); j++)
            {
                if (args[i][j] == 'a')
                {
                    to_show_hidden = true;
                }
                else if (args[i][j] == 'l')
                {
                    to_show_long_format = true;
                }
            }
        }

        else // its a directory file name just push it to directories list

        {
            directories.push_back(args[i]);
        }
    }

    // if no directories are given we will list the current working directory info i mean files
    if (directories.empty())
    {
        directories.push_back(".");
    }

    // now we will list the contents of each directory in directories vector we have taken
    for (const string &dirname : directories)
    {
        string path = dirname;
        if (path == "~")
            path = shell_home_dir;

        // NEW: First, get information about the path itself.
        struct stat path_stat;
        if (stat(path.c_str(), &path_stat) != 0)
        {
            // NEW: Handle cases where the file or directory does not exist.
            perror(("ls: cannot access '" + path + "'").c_str());
            continue;
        }

        // NEW: Check if the path is a directory.
        if (S_ISDIR(path_stat.st_mode))
        {
            // NEW: This block now only runs for directories.
            DIR *dir = opendir(path.c_str());
            if (dir == NULL)
            {
                // if we are not able to open the directory it might be a file so we will print its stats
                perror(("ls can not currently able to open the directory" + path).c_str());
                continue;
            }

            // now we will print the directory if more than one is listed
            if (directories.size() > 1)
            {
                cout << dirname << ":" << endl;
            }
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                string entry_name = entry->d_name;

                if (!to_show_hidden && entry_name[0] == '.')
                {
                    continue; // means -a is not present so qwe can skip hidden files
                }
                if (to_show_long_format)
                {
                    printfilestats(path + "/" + entry_name, entry_name);
                }
                else
                {
                    cout << entry_name << endl;
                }
            }
            closedir(dir);
            if (directories.size() > 1)
            {
                cout << endl; // we will  print a new line after each directory name  if more than one directory is listed here
            }
        }
        else
        {
            //  if the path is a file, not a directory.
            if (to_show_long_format)
            {
                // we just print the stats for the file itself.
                printfilestats(path, path);
            }
            else
            {
                // we just print the name of the file.
                cout << path << endl;
            }
        }
    }
}
//======================================= PROCESSINFO=========================================
void handle_processinfo(const vector<char *> &args)
{
    pid_t pid;
    if (args.size() == 2) // only pinfo is given show the current process i.e args vector contains {"pinfo", NULL}
    {
        pid = getpid();
    }
    // if pinfo<pid> is given we will show the info of that process having that pid
    else if (args.size() == 3)
    {
        pid = stoi(args[1]);
    }
    else
    {
        cerr << "pinfo: invalid number of arguments provided in the command" << endl;
        return;
    }
    cout << "pid -- : " << pid << endl;

    // ==========stat info of the file===========================

    string stat_path = "/proc/" + to_string(pid) + "/stat";
    FILE *stat_file = fopen(stat_path.c_str(), "r");
    if (stat_file == NULL)
    {
        perror("fopen stat file");
        return;
    }

    // now to know about the process state(running , sleeping , zombie etc) and memory usages
    char process_state;
    long unsigned int memory; // show the virtual memeory usage

    // skupping the unnecesary fields
    fscanf(stat_file, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu", &process_state, &memory);
    fclose(stat_file);

    cout << "Process Status -- {" << process_state << "}" << endl;
    cout << "memory -- " << memory << "  {Virtual Memory}" << endl;

    // ==========exe path of the file===========================

    string execution_path_str = "/proc/" + to_string(pid) + "/exe";
    char execution_path[PATH_MAX];
    ssize_t len = readlink(execution_path_str.c_str(), execution_path, sizeof(execution_path) - 1);
    if (len != -1)
    {
        execution_path[len] = '\0'; // terminating with null
        string exec_path_to_print = execution_path;
        // replace with ~ if exec path starts with  shell's home directory for radability
        if (exec_path_to_print.find(shell_home_dir) == 0)
        {
            exec_path_to_print.replace(0, shell_home_dir.length(), "~");
        }
        cout << "executable path -- : " << exec_path_to_print << endl;
    }
    else
    {
        perror("error in reading the file exepath");
    }
}
// ================built in commands implementation==============================

void handle_parentworkingdirectory()
{
    char cwd_buffer[PATH_MAX];
    if (getcwd(cwd_buffer, sizeof(cwd_buffer)) == nullptr)
    {
        perror("getcwd");
        return;
    }
    cout << cwd_buffer << endl;
}

// for echo
void handle_echo(const vector<char *> &args)
{
    // start from index1 and print everything
    for (size_t i = 1; i < args.size() - 1; i++)
    {
        cout << args[i] << (i == args.size() - 2 ? "" : " ");
    }
    cout << endl;
}

// now to handle cd function call
void handle_cd(const vector<char *> &args)
{
    if (args.size() > 3)
    {
        cerr << "cd:too many args passed" << endl;
        return;
    }
    char current_directory_buffer[PATH_MAX];
    getcwd(current_directory_buffer, sizeof(current_directory_buffer));
    const char *path;
    if (args.size() == 2 || strcmp(args[1], "~") == 0)
    {
        path = shell_home_dir.c_str();
    }
    else if (strcmp(args[1], "-") == 0)
    {
        if (prev_dir.empty())
        {
            cerr << "cd:oldpath not set." << endl;
            return;
        }
        path = prev_dir.c_str();
        cout << path << endl;
    }
    else
    {
        path = args[1];
    }
    if (chdir(path) != 0)
    {
        perror("cd");
    }
    else
    {
        prev_dir = current_directory_buffer;
    }
}

//=============================== execute piped commands=======================================

// Replace your entire executepipedcommands function with this
void executepipedcommands(vector<vector<char *>> &commands, const string &inputfile, const string &outputfile, bool isappend)
{
    int num_of_commands = commands.size();
    if (num_of_commands == 0)
        return;

    int input_fd = STDIN_FILENO; //  input fd will track what the next child should read from
    int pipe_fd[2];              // it holds the read and write ends of the pipe

    for (int i = 0; i < num_of_commands; i++)
    {
        if (i < num_of_commands - 1)
        {
            if (pipe(pipe_fd) == -1)
            {
                perror("pipe");
                return;
            }
        }
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            return;
        }

        if (pid == 0) // Child Process
        {
            // reset the signal handling to default for child process so that it can respond to ctrl+c and ctrl+z
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            // === for the input redirection part======
            if (i == 0 && !inputfile.empty())
            {
                // first command gets input from file if specified i.e if in pipe_fd
                int in_fd = open(inputfile.c_str(), O_RDONLY);
                if (in_fd == -1)
                {
                    perror("open input");
                    exit(EXIT_FAILURE);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            else if (input_fd != STDIN_FILENO)
            {
                // else if it is npot the 1st command so it   get input from previous pipe
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            // === output redirect===
            if (i == num_of_commands - 1 && !outputfile.empty())
            {
                // Last command sends output to file if specified in the array pipe_fd
                int out_flags = O_WRONLY | O_CREAT | (isappend ? O_APPEND : O_TRUNC);
                int out_fd = open(outputfile.c_str(), out_flags, 0644);
                if (out_fd == -1)
                {
                    perror("open output");
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }
            else if (i < num_of_commands - 1)
            {
                // so its  not the last command so send output to next pipe
                close(pipe_fd[0]); // we don't need to read from the new pipe
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
            }

            // now for the built in commands we will handle them here itself we will not go to execvp
            string command_name = commands[i][0];
            if (command_name == "pwd" || command_name == "echo" || command_name == "ls" || command_name == "pinfo" || command_name == "history" || command_name == "search")
            {
                if (command_name == "pwd")
                {
                    handle_parentworkingdirectory();
                }
                else if (command_name == "echo")
                {
                    handle_echo(commands[i]);
                }

                else if (command_name == "ls")
                {
                    handlelscomm(commands[i]);
                }
                else if (command_name == "pinfo")
                {
                    handle_processinfo(commands[i]);
                }
                else if (command_name == "history")
                {
                    handle_history(commands[i]);
                }
                else if (command_name == "search")
                {
                    handle_search(commands[i]);
                }
                exit(0); // exit the child process after handling the built in command
            }

            // Execute the command means replace the child process with the command i.e the actual one
            if (execvp(commands[i][0], commands[i].data()) == -1)
            {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
        else // parent rocess
        {
            if (input_fd != STDIN_FILENO)
                close(input_fd);
            if (i < num_of_commands - 1)
            {
                close(pipe_fd[1]);
                input_fd = pipe_fd[0];
            }
        }
    }
    for (int i = 0; i < num_of_commands; i++)
    {
        wait(NULL);
    }
}



// =================function to execute a  external command ============================
void executeexternal(vector<char *> &args, bool is_background, const string &inputfile, const string &outputfile, bool isappend)
{

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        //     delete[] c_command;
        return;
    }
    else if (pid == 0) // execute in child process with the command args[0]
    {

        if (!is_background)
        {
            // foreground process should respond to ctrl+c and ctrl+z signals
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
        }
        // input redirection logic
        if (!inputfile.empty())
        {
            int input_fd = open(inputfile.c_str(), O_RDONLY);
            if (input_fd == -1)
            {
                perror("open input file");
                exit(EXIT_FAILURE);
            }
            if (dup2(input_fd, STDIN_FILENO) == -1) // redirect the ip to the file instead of keyboard
            {
                perror("dup2 input file");
                exit(EXIT_FAILURE);
            }
            close(input_fd); // close the original file descriptor
        }

        // output redirection logic
        if (!outputfile.empty())
        {

            int output_flags = O_WRONLY | O_CREAT | (isappend ? O_APPEND : O_TRUNC);
            int output_fd = open(outputfile.c_str(), output_flags, 0644); //
            if (output_fd == -1)
            {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            // redirect the stdout to the file instead of terminal
            if (dup2(output_fd, STDOUT_FILENO) == -1)
            {
                perror("dup2 output file");
                exit(EXIT_FAILURE);
            }
            close(output_fd); // close the original file descriptor
        }

        // child process and now to execute the command with redirected i/o
        if (execvp(args[0], args.data()) == -1)
        {
            perror("execvp");   // if execvp returns it means error
            exit(EXIT_FAILURE); // exit the child process
        }
    }
    else // pid >0 i.e paren process
    {
        if (is_background)
        {
            cout << "[" << pid << "]" << endl;
        }
        else
        {
            int status;
            waitpid(pid, &status, WUNTRACED); // when we press Ctrl+Z the foreground process is stopped and not terminated
            //.hence the parent will infinitely wait for the waitpid i.e waiting forever for a termination that will never happen..so we use WUNTRACED flag
            // so that tells the operating system: "Wake me up if the child process either terminates OR is stopped."
        } // parent process calls the waitpid passing the child pid if foreground process
    }
    // delete[] c_command; // clean up the memory i.e allocate for c_command
}
int main()
{
    // store the starting directory of the shell
    char home_dir[PATH_MAX];
    if (getcwd(home_dir, sizeof(home_dir)) == nullptr)
    {
        perror("getcwd at startup");
        return 1;
    }
    shell_home_dir = home_dir;
    prev_dir = " ";
    tcgetattr(STDIN_FILENO, &original_termios);
    atexit(disableRawMode);
    load_command_history(); // load the history of commands from the history file

    // signal handling for ctrl+c and ctrl+z i.e our shell should ignore these signals
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    // enabble the raw mode to read char by char input
    enablerawmode();
    // REPL LOOP
    string full_command;
    while (true)
    {
        enablerawmode();
        displaytheprompt();
        cout << flush;
        full_command = takeinput();
        // 2. IMMEDIATELY RESTORE NORMAL MODE
        disableRawMode();

        if (!full_command.empty())
        {
            // Add the command to history deque
            command_history.push_back(full_command);
            if (command_history.size() > MAX_HISTORY_SIZE)
            {
                command_history.pop_front(); // remove the oldest command if we exceed max size 20
            }
        }
        if (full_command.empty())
        {
            continue;
        }
        if (full_command == "exit")
        {
            break; // exit the shell
        }

        // as both strtok use a global pointer so after one inner strtok runs the the outerone finds garbage when it resumes . so we will use 2 pointer instead one global one
        char *saveptr_outer;

        // for strtok we need a non const string
        char *c_command_full = new char[full_command.length() + 1];
        strcpy(c_command_full, full_command.c_str());
        char *single_command = strtok_r(c_command_full, ";", &saveptr_outer); // split the line of commands by semicolon

        while (single_command != NULL)
        {

            vector<char *> commands_string; // to hold the piped commands
            char *saveptr_pipe;
            char *piped_command = strtok_r(single_command, "|", &saveptr_pipe);
            while (piped_command != NULL)
            {
                commands_string.push_back(piped_command);
                piped_command = strtok_r(NULL, "|", &saveptr_pipe);
            }
            if (commands_string.size() > 1)
            {
                string inputfile = "", outputfile = "";
                bool isappend = false;
                vector<vector<char *>> commands; // to hold the clean commands
                for (size_t i = 0; i < commands_string.size(); i++)
                {
                    vector<char *> temptokens;
                    char *saveptrtemp;
                    char *token = strtok_r(commands_string[i], " \t\n", &saveptrtemp); // split the single command by space or tab
                    while (token != NULL)
                    {
                        temptokens.push_back(token);
                        token = strtok_r(NULL, " \t\n", &saveptrtemp);
                    }

                    vector<char *> cleanargs; // this will have the cleanm commands
                    if (i == 0)
                    {
                        for (size_t j = 0; j < temptokens.size(); j++)
                        {
                            if (strcmp(temptokens[j], "<") == 0)
                            {
                                if (j + 1 < temptokens.size())
                                {
                                    inputfile = temptokens[j + 1];
                                    j++; // skip the next as it is the filename
                                }
                            }
                            else
                            {
                                cleanargs.push_back(temptokens[j]);
                            }
                        }
                    }
                    else if (i == commands_string.size() - 1) // now checking for the op redirecton
                    {
                        for (size_t j = 0; j < temptokens.size(); j++)
                        {
                            if (strcmp(temptokens[j], ">") == 0)
                            {
                                if (j + 1 < temptokens.size())
                                {
                                    outputfile = temptokens[j + 1];
                                    isappend = false;
                                    j++; // skip the next token as it is the filename
                                }
                            }
                            else if (strcmp(temptokens[j], ">>") == 0) // for append mode
                            {
                                if (j + 1 < temptokens.size())
                                {
                                    outputfile = temptokens[j + 1];
                                    isappend = true;
                                    j++; // skip the next token as it is the filename
                                }
                            }
                            else
                            {
                                cleanargs.push_back(temptokens[j]);
                            }
                        }
                    }
                    else
                    {
                        cleanargs = temptokens; // middle commands will have no redirection
                    }
                    cleanargs.push_back(NULL); // null terminate the args for execvp
                    if (!cleanargs.empty() && cleanargs[0] != NULL)
                    {
                        commands.push_back(cleanargs);
                    }
                }
                executepipedcommands(commands, inputfile, outputfile, isappend);
            }
            else // for command with no pipes
            {
                vector<char *> tokens;
                char *saveptr_inner;
                char *token = strtok_r(single_command, " \t\n", &saveptr_inner); // split the single command by space or tab
                while (token != NULL)
                {
                    tokens.push_back(token);
                    token = strtok_r(NULL, " \t\n", &saveptr_inner);
                }
                if (tokens.empty())
                {
                    single_command = strtok_r(NULL, ";", &saveptr_outer); // get the next command if token is non empty
                    continue;
                }

                string inputfile = "", outputfile = "";
                bool isappend = false;
                vector<char *> args; // this will have the cleanm command arguments
                for (size_t i = 0; i < tokens.size(); i++)
                {
                    if (strcmp(tokens[i], "<") == 0)
                    {
                        if (i + 1 < tokens.size())
                        {
                            inputfile = tokens[i + 1];
                            i++; // skip the next token as it is the filename
                        }
                    }
                    else if (strcmp(tokens[i], ">") == 0)
                    {
                        if (i + 1 < tokens.size())
                        {
                            outputfile = tokens[i + 1];
                            isappend = false;
                            i++; // skip the next token as it is the filename
                        }
                    }
                    else if (strcmp(tokens[i], ">>") == 0) // for append mode
                    {
                        if (i + 1 < tokens.size())
                        {
                            outputfile = tokens[i + 1];
                            isappend = true;
                            i++; // skip the next token as it is the filename
                        }
                    }
                    else
                    {
                        args.push_back(tokens[i]);
                    }
                }

                bool is_background = false;
                if (!args.empty() && strcmp(args.back(), "&") == 0)
                {
                    is_background = true;
                    args.pop_back();
                }
                // execvp requrires a null terminated array of char*
                args.push_back(NULL);

                // if there are no arguments, return
                if (args.empty() || args[0] == NULL)
                {

                    single_command = strtok_r(NULL, ";", &saveptr_outer); // get the next command
                    continue;
                }
                string command_name = args[0];
                if (command_name == "pwd" || command_name == "echo" || command_name == "cd" || command_name == "pinfo" || command_name == "search" || command_name == "ls" || command_name == "history")
                {
                    if (command_name == "cd")
                    {
                        handle_cd(args);
                    }
                    else
                    {
                        int saved_standard_output = dup(STDOUT_FILENO);
                        if (!outputfile.empty())
                        {
                            int output_flags = O_WRONLY | O_CREAT | (isappend ? O_APPEND : O_TRUNC);
                            int output_fd = open(outputfile.c_str(), output_flags, 0644);
                            if (output_fd == -1)
                                perror("open output file");
                            else
                            {
                                dup2(output_fd, STDOUT_FILENO);
                                close(output_fd);
                            }
                        }

                        // NOW TO EXECUTE THE EXTERNAL COMMANDS
                        if (command_name == "pwd")
                            handle_parentworkingdirectory();
                        else if (command_name == "echo")
                            handle_echo(args);
                        else if (command_name == "pinfo")
                            handle_processinfo(args);
                        else if (command_name == "search")
                            handle_search(args);
                        else if (command_name == "ls")
                            handlelscomm(args);
                        else if (command_name == "history")
                            handle_history(args);
                        // restore the original stdout

                        dup2(saved_standard_output, STDOUT_FILENO);
                        close(saved_standard_output);
                    }
                }
                else
                {
                    executeexternal(args, is_background, inputfile, outputfile, isappend);
                }
            }
            // Move to the next command in the outer loop
            single_command = strtok_r(NULL, ";", &saveptr_outer);
        }

        delete[] c_command_full; // free the memory of the full line of command
        cout << endl; 
    }
    save_command_history(); // save the history to the history file after executing each command

    return 0;
}