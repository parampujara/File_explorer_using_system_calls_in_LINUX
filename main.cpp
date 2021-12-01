#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <termios.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <string>
#include <dirent.h>
#include <algorithm>

using namespace std;

// global variables to be used throughout the program
static struct termios initSettings, newSettings;
static int p_char = -1;
char *root_path;
size_t direct_s = 1024;
char currdir[1024];
#define MAX 10 // max items to display in normal mode
struct winsize w;
vector<string> inputVector,backStack,forwardStack,files,searchStack;
int starting = 0; 
int ending = starting + MAX;
int cursor = 1;
int cline = MAX + 2;
int searchflag = 1;
int x = 1, y = 0;
FILE *out;

void clr()
{
	printf("\033[H\033[J");
}
void gotocursor(int x, int y)
{
	cout << "\033[" << x << ";" << y << "H";
	fflush(stdout);
}
void noncanonical()
{
	tcgetattr(0, &initSettings);
	newSettings = initSettings;
	newSettings.c_lflag &= ~(ECHO | ICANON);
	newSettings.c_cc[VMIN] = 1;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &newSettings);
	return;
}
void canonical()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &initSettings);
	return;
}


void display(const char *dirName, const char *root)
{
	string finalpath;
	if (searchflag == 1)
	{
		finalpath = string(dirName);
	}
	else
	{
		finalpath = string(root) + "/" + string(dirName);
	}
	char *path = new char[finalpath.length() + 1];
	strcpy(path, finalpath.c_str());
	struct stat sb;
	
	if (stat(path, &sb) == -1)
	{
		perror("lstat");
	}
	cout << " ";
	struct passwd *pw = getpwuid(sb.st_uid);
	struct group *gr = getgrgid(sb.st_gid);
	if(S_ISDIR(sb.st_mode))
	{
		printf("\033[1;32m");
		printf("%-50s\t", dirName);
		printf("\033[0m");
	}
	else
		printf("%-50s\t", dirName);
	
	if((S_ISDIR(sb.st_mode)))
	{
		cout<<"d";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IRUSR))
	{
		cout<<"r";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IWUSR))
	{
		cout<<"w";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IXUSR))
	{
		cout<<"x";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IRGRP))
	{
		cout<<"r";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IWGRP))
	{
		cout<<"w";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IXGRP))
	{
		cout<<"x";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IROTH))
	{
		cout<<"r";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IWOTH))
	{
		cout<<"w";
	}
	else
	{
		cout<<"-";
	}
	if((sb.st_mode & S_IXOTH))
	{
		cout<<"x";
	}
	else
	{
		cout<<"-";
	}

	
	if (pw != 0)
		printf("\t%-8s", pw->pw_name);
	if (gr != 0)
		printf(" %-8s", gr->gr_name);

	double size_file=((double)sb.st_size) / 1024;
	char *tt = (ctime(&sb.st_mtime));
	tt[strlen(tt) - 1] = '\0';


	printf("%10.2fKB", size_file);
	printf("%30s", tt);
	cout<<endl;
}

void printhere()
{
	string filenamef;
	for (int i = starting; i < ending && i < files.size();i++)
	{
		filenamef = files[i];
		char *c = strcpy(new char[filenamef.length() + 1], filenamef.c_str());
		display(c, root_path);
	}
	gotocursor(cline, 0);
	cout << "NORMAL MODE" << endl;
	gotocursor(cursor, 0);
}

int readCh()
{
	char a;
	if(p_char == -1)
	{
		read(0, &a, 1);
	}
	else if (p_char != -1)
	{
		a = p_char;
		p_char = -1;
	}
	return a;
}

void go_up()
{
	if(cursor > 1)
	{
		cursor--;
		gotocursor(cursor, 0);
		return;
	}
	if (starting == 0)
		return;
	starting--;
	ending--;
	gotocursor(1, 0);
	printhere();
	return;
}
void go_down()
{
	if(cursor < files.size() )
	{
		if(cursor < MAX)
		{
			cursor++;
			gotocursor(cursor, 0);
			return;
		}
		else
		{
			
		}
	}
	else
	{

	}
	if (ending == files.size())
		return;
	starting++;
	ending++;
	gotocursor(1, 0);
	printhere();
	return;
}

void opendirect(const char *path)
{
	clr();
	cursor = 1;
	DIR *dp;
	struct dirent *entry;

	if (!(dp = opendir(path)))
	{
		fprintf(stderr, "Can't open!\n");
		return;
	}
	chdir(path);
	getcwd(currdir, direct_s);
	files.clear();
	while ((entry = readdir(dp)) != NULL)
	{
		if (string(entry->d_name) == ".." && !strcmp(currdir, root_path))
		{
			gotocursor(22, 0);
			cout << "It's Root.";
			gotocursor(cursor, 0);
		}
		else
			files.push_back(string(entry->d_name));
	}
	closedir(dp);
	starting = 0;
	ending = min(MAX, int(files.size()));
	sort(files.begin(), files.end());
	printhere();
	gotocursor(cursor, 0);
}

void up_func()
{
	if (strcmp(currdir, root_path)!=0)
	{
		backStack.push_back(string(currdir));
		opendirect("../");
	}
}
void home_func()
{
	if (strcmp(currdir, root_path)!=0)
	{
		backStack.push_back(string(currdir));
		opendirect(root_path);
	}
}
void back_func()
{
	if (backStack.size()!=0)
	{
		string toVisit = backStack.back();
		backStack.pop_back();
		forwardStack.push_back(string(currdir));
		opendirect(toVisit.c_str());
	}
	
}
void forward_func()
{
	if (forwardStack.size()!=0)
	{
		string toVisit = forwardStack.back();
		forwardStack.pop_back();
		backStack.push_back(string(currdir));
		opendirect(toVisit.c_str());	
	}
}

void commandbreaking(string command)
{
	inputVector.clear();
	int i = 0;
	int n = command.size();
	string word;
	while (i < n)
	{
		word = "";
		while (command[i] != ' ' && i < n)
		{
			if (command[i] != 92)
			{
				word += command[i];
				i++;
			}
			else
			{
				word += " ";
				i = i + 2;
			}
			//cout<<"i am here for debug"<<endl;
		}
		inputVector.push_back(word);
		i++;
	}
}


void file_search(string fileName, string dir)
{
	DIR *dp;
	

	if((dp = opendir(dir.c_str())) != NULL)
	{
		struct dirent *entry;
		struct stat statbuf;
		chdir(dir.c_str()); //  i m in a
		while ((entry = readdir(dp)) != NULL)
		{
			lstat(entry->d_name, &statbuf);
			if (S_ISDIR(statbuf.st_mode)!=0)
			{
				if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
				{
					continue;
				}
				file_search(fileName, entry->d_name);
			}
			else if(S_ISDIR(statbuf.st_mode)==0)
			{
				int offset = string(root_path).size();
				if (strcmp(fileName.c_str(), entry->d_name) != 0)
				{
					offset = string(root_path).size();
				}
				else
				{
					char buf[1000];
					getcwd(buf, 1000);
					string toPut = string(buf);
					searchStack.push_back(toPut.substr(offset) + '/' + string(entry->d_name));
				}
			}
		}
		chdir("..");
		closedir(dp);
	}
	else if ((dp = opendir(dir.c_str())) == NULL)
	{
		fprintf(stderr, "Can't open the directory: %s\n", dir.c_str());
	}
	
}



int file_delete(string filePath)
{
	return unlink(filePath.c_str());
}

void file_copy(string fileName, string dest)
{
	int nread;
	char block[1024];
	
	int in = open(fileName.c_str(), O_RDONLY);
	string temp =dest + '/' + fileName;
	int out = open(temp.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	while ((nread = read(in, block, sizeof(block))) > 0)
	{
		write(out, block, nread);
	}
	
}

void dir_copy(string dir, string dest)
{
	DIR *dp;
	if((dp = opendir(dir.c_str())) != NULL)
	{
		
		struct dirent *entry;
		struct stat statbuf;
		chdir(dir.c_str()); 
		while ((entry = readdir(dp)) != NULL)
		{
			lstat(entry->d_name, &statbuf);
			if (S_ISDIR(statbuf.st_mode))
			{
				if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0)
				{
					mkdir((dest + '/' + entry->d_name).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
					dir_copy(entry->d_name, dest + '/' + entry->d_name);
				}
				else
				{
					continue;
				}				
			}
			else
			{
				file_copy(entry->d_name, dest);
			}
		}
		chdir("..");
		closedir(dp);

	}
	else
	{
		printf("Can't open the directory: %s", dir.c_str());
	}
	
}

void command_mode()
{
	string command;
	gotocursor(cline, y);
	cout << "COMMAND MODE";
	gotocursor(cline + 1, y);
	canonical();
	cout << "Enter Command:";
	command = "start";

	commandbreaking(command);
	while (inputVector[0] != "exit")
	{
		getline(cin, command);
		//cin>>command;
		commandbreaking(command);
		if (inputVector[0] == "copy_dir")
		{
			string dirName, dest;
			int size = inputVector.size();
			if(inputVector[size - 1][0] != '/' && inputVector[size - 1][0] != '~')
			{
				gotocursor(cline + 2, 0);
				cout << "Invalid Path!                                                                                       ";
				gotocursor(cline + 1, y);
				cout << "                                                                                                            ";
				gotocursor(cline + 1, y);
				cout << "Enter Command:";
				continue;
			}
			else if (inputVector[size - 1][0] == '/')
			{
				dest = currdir + inputVector[size - 1];
			}
			else if (inputVector[size - 1][0] == '~')
			{
				dest = root_path + inputVector[size - 1].substr(1);
			}
			else
			{
				
			}
			
			
			for (int i = 1; i < size - 1; i++)
			{
				dirName = inputVector[i];
				mkdir((dest + '/' + dirName).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
				dir_copy(currdir + '/' + dirName, dest + '/' + dirName);
			}

			gotocursor(cline + 2, y);
			cout << "Copied Succesfully!          ";
		}
		else if (inputVector[0] == "move")
		{
			string fileName, dest;
			int size = inputVector.size();
			if (inputVector[size - 1][0] != '~' && inputVector[size - 1][0] != '/')
			{
				gotocursor(cline + 2, 0);
				cout << "Invalid Path!                                                                                       ";
				gotocursor(cline + 1, y);
				cout << "                                                                                                            ";
				gotocursor(cline + 1, y);
				cout << "Enter Command:";
				continue;
			}
			else if (inputVector[size - 1][0] == '~')
			{
				dest = root_path + inputVector[size - 1].substr(1);
			}
			else if (inputVector[size - 1][0] == '/')
			{
				dest = currdir + inputVector[size - 1];
			}
			else
			{
				
			}
			for (int i = 1; i < size - 1; i++)
			{
				fileName = inputVector[i];
				file_copy(fileName, dest);
				file_delete(currdir + '/' + fileName);
			}
			opendirect(currdir);

			gotocursor(cline + 2, y);
			cout << "moved Succesfully!          ";
			gotocursor(cline + 1, 0);
			cout << "Enter Command:";
		}
		else if (inputVector[0] == "copy")
		{
			string fileName, dest;
			int size = inputVector.size();
			if (inputVector[size - 1][0] != '~' && inputVector[size - 1][0] != '/')
			{
				gotocursor(cline + 2, 0);
				cout << "Invalid Path!                                                                                       ";
				gotocursor(cline + 1, y);
				cout << "                                                                                                            ";
				gotocursor(cline + 1, y);
				cout << "Enter Command:";
				continue;
			}
			else if (inputVector[size - 1][0] == '~')
			{
				dest = root_path + inputVector[size - 1].substr(1);
			}
			else if (inputVector[size - 1][0] == '/')
			{
				dest = currdir + inputVector[size - 1];
			}
			else
			{
				
			}
			int i=1;
			while (i<size-1)
			{
				fileName = inputVector[i];
				file_copy(fileName, dest);
				i++;
			}
			
			gotocursor(cline + 2, y);
			cout << "Copied Succesfully!          ";
		}
		else if (inputVector[0] == "rename")
		{
			if (inputVector.size() == 3)
			{
				string a = inputVector[1];
				string b = inputVector[2];
				rename(a.c_str(), b.c_str());
				gotocursor(cline + 2, 0);
				cout << "Renamed Succesfully!";
				gotocursor(1, 0);
				opendirect(currdir);
				gotocursor(cline + 1, y);
				cout << "Enter Command:";
			}
			else
			{
				gotocursor(cline + 2, 0);
				cout << "Command Invalid!!";
			}
		}
		else if (inputVector[0] == "create_file" || inputVector[0] == "create_dir")
		{
			string loc;
			int size = inputVector.size();
			if (inputVector[size - 1][0] == '.')
			{
				loc = currdir;
			}
			else if (inputVector[size - 1][0] == '~')
			{
				loc = root_path + inputVector[size - 1].substr(1);
			}
			else if (inputVector[size - 1][0] == '/')
			{
				loc = currdir + inputVector[size - 1];
			}
			else
			{
				gotocursor(cline + 2, 0);
				cout << "Invalid Path!                                                                                       ";
				gotocursor(cline + 1, y);
				cout << "                                                                                                            ";
				gotocursor(cline + 1, y);
				cout << "Enter Command:";
				continue;
			}
			for (int i = 1; i < size - 1; i++)
			{
				string fileName = inputVector[i];
				if (inputVector[0] == "create_file")
				{
					open((loc + '/' + fileName).c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
				}
				else
				{
					mkdir((loc + '/' + fileName).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
				}
			}
			gotocursor(1, 0);
			opendirect(currdir);
			gotocursor(cline + 2, 0);
			cout << inputVector[0] << " implemented!!";
		}
		else if (inputVector[0] == "delete_file")
		{
			string filePath;
			if (inputVector[1][0] == '~')
			{
				filePath = root_path + inputVector[1].substr(1);
			}
			else if (inputVector[1][0] == '/')
			{
				filePath = currdir + inputVector[1];
			}
			else
			{
				gotocursor(cline + 2, 0);
				cout << "Give proper path!                                                                                       ";
				gotocursor(cline + 1, y);
				cout << "                                                                                                            ";
				gotocursor(cline + 1, y);
				cout << "Enter Command:";
				continue;
			}
			
			gotocursor(cline + 2, 0);
			int opt=file_delete(root_path + '/' + filePath);
			if (opt==0)
			{
				gotocursor(cline + 2, 0);
				cout << "Deleted Succesfully!                          ";
			}
			else
			{
				gotocursor(cline + 2, 0);
				cout << "Deletion Failed!                               ";
			}
			opendirect(currdir);
			gotocursor(cline + 2, 0);
			cout << inputVector[0] << " implemented!!";
		}
		else if (inputVector[0] == "goto")
		{
			if (inputVector[1][0] == '/')
			{
				if (strcmp(currdir, root_path)!=0)
				{
					backStack.push_back(string(currdir));
					opendirect(root_path);
				}
			}
			else
			{
				if (inputVector[1][0] == '~')
				{
					opendirect((root_path + inputVector[1].substr(1)).c_str());
				}
			}
			gotocursor(cline + 1, 0);


			gotocursor(cline + 2, y);
			cout << inputVector[0] << " implemented!!";
		}
		else if (inputVector[0] == "search")
		{
			string fileName = inputVector[1];
			searchStack.clear();
			file_search(fileName, currdir);
			opendirect(currdir);
			if (searchStack.size())
			{
				gotocursor(cline, y);
				cout << "COMMAND MODE";
				gotocursor(cline + 2, y);
				cout << "                                                    ";
				gotocursor(cline + 4, y);
				cout << inputVector[1] << " is found";
			}
			else
			{
				gotocursor(cline, y);
				cout << "COMMAND MODE";
				gotocursor(cline + 2, y);
				cout << "                                                    ";
				gotocursor(cline + 4, y);
				cout << inputVector[1] << " is not found";
			}
		}
		else
		{
			gotocursor(cline + 2, y);
			cout << "Command was invalid!";
		}
		gotocursor(cline + 1, y);
		cout << "                                                                                                            ";
		gotocursor(cline + 1, y);
		cout << "Enter Command:";
	}
	noncanonical();
	gotocursor(cline + 4, y);
	cout << "                                                                                                         ";
	gotocursor(cline, y);
	cout << "NORMAL MODE ";
	gotocursor(cline + 1, y);
	cout << "                                                                                                            ";
	gotocursor(cline + 2, y);
	cout << "                                                                                                            ";
	cursor = 1;
	gotocursor(cursor, y);
}
int fast_input()
{
	char ch;
	int nread;
	if (p_char != -1)
	{
		return 1;
	}
		
	else
	{
		newSettings.c_cc[VMIN] = 0;
		tcsetattr(0, TCSANOW, &newSettings);
		nread = read(0, &ch, 1);
		newSettings.c_cc[VMIN] = 1;
		tcsetattr(0, TCSANOW, &newSettings);
		if (!nread)
		{
			return 0;
		}
		else
		{
			p_char = ch;
			return 1;
		}
	}
	return 1;
}

void file_open()
{

	struct stat statbuf;
	string temp = files[cursor + starting - 1];
	gotocursor(20, 0);
	cout << temp;
	gotocursor(cursor, 0);
	char t[temp.size() + 1];
	strcpy(t, temp.c_str());
	char *fileName = t;
	lstat(fileName, &statbuf);

	
	if (S_ISDIR(statbuf.st_mode))
	{
		if (temp == ".." && strcmp(currdir, root_path) == 0)
			return;
		if (temp == ".")
			return;
		backStack.push_back(string(currdir));

		opendirect((string(currdir) + '/' + string(fileName)).c_str());
	}
	else
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			execl("/usr/bin/xdg-open", "xdg-open", fileName, NULL);
			exit(1);
		}
	}

	return;
}

int main()
{
	clr();
	char path[1024];
	size_t size = 1024;
	getcwd(path, size);
	root_path = path;  // set the root path for program
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	opendirect(path);
	noncanonical(); // initialise terminal settings

	char ch = 'a';
	while (ch != 'q')
	{
		if (fast_input())
		{
			ch = readCh(); // take a character as user input and respond instantly

			if (ch == 65)
			{
				go_up();
			}
			else if (ch == 66)
			{
				go_down();
			}
			else if (ch == 67)
			{
				forward_func();
			}
			else if (ch == 68)
			{
				back_func();
			}
			else if (ch == 127)
			{
				up_func();
			}
			else if (ch == 10)
			{
				file_open();
				cursor = 1;
				gotocursor(cursor, 0);
			}
			else if (ch == 'h')
			{
				home_func();
			}
			else if (ch == ':')
			{
				command_mode();
			}
			else
			{
			}
		}
		gotocursor(21, 0);
		cout << "            ";
		gotocursor(21, 0);
		cout << "Cursor:" << cursor;
		gotocursor(cursor, 0);
	}

	clr();
	canonical(); // restore terminal settings
	exit(0);
}