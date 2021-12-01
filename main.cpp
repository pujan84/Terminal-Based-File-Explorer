#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <grp.h>
#include <sys/types.h>
#include <pwd.h>
#include <iomanip>
#include <sys/stat.h>
#include <dirent.h>
#include <termios.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string>
#include <sys/wait.h>
#include <bits/stdc++.h>
#define esc 27

using namespace std;

char *root;
char *current_Path;
int search_Flag = 0;
vector<string> d_List;
stack<string> backward_stack;
stack<string> forward_stack;
int row_size, column_size;
vector<string> search_vector;
int total_NoofFiles;
int cor_X = 1, cor_Y = 1;

vector<string> input_list;

void open_Dir(const char *path);
void print_fun(const char *d_name, const char *root);
int get_Filecount();
void do_navigation();

int is_directory(char *newpath);

int command_mode();
void clear_screen();
string get_absolutepath(string s);
void create_mulfiles(vector<string> v);
void create_onefile(char *create_path);
void make_Dir(vector<string> v);
void remove_muldir(vector<string> v);
void remove_onedir(char *rm_path);
void remove_mulfiles(vector<string> v);
void remove_onefile(char *rm_path);
void rename_file(vector<string> v);
void copy_command(vector<string> v);
void copy_directory(char *o_path, char *d_path);
void copy_File(char *o_path, char *d_path);
void move(vector<string> v);

int search_cmd(vector<string> v);
void recursive_search(char *s_path, string s_name);

void remove_onedir(char *rm_path)
{
	struct dirent *d;
	DIR *directory;
	directory = opendir(rm_path);
	if (!directory)
	{
		cout << endl;
		cout << ":";
	}
	else
	{
		while ((d = readdir(directory)) != NULL)
		{
			if ((string(d->d_name) != "..") && (string(d->d_name) != "."))
			{

				string temp_path = string(rm_path) + "/";
				temp_path+= string(d->d_name);
				char *l_path = new char[temp_path.length() + 1];
				strcpy(l_path, temp_path.c_str());

				struct stat s;
				if (stat(l_path, &s) != -1)
				{

					if ((S_ISDIR(s.st_mode)))
					{
						remove_onedir(l_path);
					}
					else
					{
						remove_onefile(l_path);
					}
				}
				else
				{
					perror("lstat");
				}
			}
		}
		closedir(directory);
		int s = rmdir(rm_path);
		if (s == -1)
		{
			cout << endl;
			cout << "Error to removing this Directory : " + string(rm_path) << endl;
			cout << ":";
		}
	}
}

void recursive_search(char *s_path, string s_name)
{
	struct dirent *d;
	DIR *directory;
	directory = opendir(s_path);
	if (!directory)
	{
		cout << endl;
		cout << "Unable to search Directory" << endl;
		cout << ":";
	}
	else
	{
		while ((d = readdir(directory)) != NULL)
		{
			if ((string(d->d_name) != "..") && (string(d->d_name) != "."))
			{
				string temp = string(d->d_name);
				string temppath = string(s_path)+"/";
				temppath+= temp;
				char *l_path = new char[temppath.length() + 1];
				strcpy(l_path, temppath.c_str());

				struct stat s;
				if (stat(l_path, &s) != -1)
				{

					if ((S_ISDIR(s.st_mode)))
					{
						if (s_name == temp)
						{
							search_vector.push_back(string(l_path));
						}
						else
						{
							recursive_search(l_path, s_name);
						}
					}
					else
					{
						if (s_name == temp)
						{
							search_vector.push_back(string(l_path));
						}
					}
				}
				else
				{
					perror("lstat");
				}
			}
		}
	}
}

int search_cmd(vector<string> v)
{
	int total = v.size();
	search_vector.clear();
	if (total == 2)
	{
		string s_fname = v[1];
		char *file_path = new char[strlen(current_Path) + 1];
		strcpy(file_path, current_Path);
		recursive_search(file_path, s_fname);

		if (search_vector.size() > 0)
		{

			search_Flag = 1;

			cout << endl;
			cout << "Yes" << endl;
			return 1;
		}

		cout << endl;
		cout << "No" << endl;
		cout << ":";
		return 0;
	}
	cout << endl;
	cout << "No of Arguments in search command is invalid" << endl;
	cout << ":";
	return 0;
}

void remove_onefile(char *rm_path)
{
	int s = remove(rm_path);
	if (s != 0)
	{
		cout << endl;
		cout << "Error Occurs while removing the file : " + string(rm_path) << endl;
		cout << ":";
	}
}

void remove_mulfiles(vector<string> v)
{
	int total = v.size();
	if (total < 2)
	{
		cout << endl;
		cout << "No of Arguments is invalid in delete_file command" << endl;
		cout << ":";
	}
	for (int i = 1; i < total; i++)
	{
		char *temp_path = new char[v[i].length() + 1];
		strcpy(temp_path, v[i].c_str());
		remove_onefile(temp_path);
	}
}

void rename_file(vector<string> v)
{
	int size = v.size();
	if (size == 3)
	{
		string old_name = v[1];
		string new_name = v[2];
		rename(old_name.c_str(), new_name.c_str());
	}
	else
	{
		cout << endl;
		cout << "Number of arguments for renaming is invalid : " << endl;
		cout << ":";
	}
}

void create_onefile(char *create_path)
{
	int s = open(create_path, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (s == -1)
	{
		cout << endl;
		cout << "Error occured while creating new file with path : " + string(create_path) << endl;
		cout << ":";
	}
}

void create_mulfiles(vector<string> v)
{
	int total = v.size();
	if (total > 2)
	{
		string tmp = get_absolutepath(v[total - 1]);
		for (int i = 1; i < total - 1; i++)
		{
			string newpath = tmp + "/";
			newpath += v[i];
			char *final_path = new char[newpath.length() + 1];
			strcpy(final_path, newpath.c_str());
			create_onefile(final_path);
		}
	}
	else
	{
		cout << endl;
		cout << "No of Arguments is invalid : " << endl;
		cout << ":";
	}
}

void remove_muldir(vector<string> v)
{
	int size = v.size();
	if (size > 1)
	{
		for (int i = 1; i < size; i++)
		{
			char *tmp = new char[v[i].length() + 1];
			strcpy(tmp, v[i].c_str());
			remove_onedir(tmp);
		}
	}
	else
	{
		cout << endl;
		cout << "Number of Arguments is invalid" << endl;
		cout << ":";
	}
}

string get_absolutepath(string s)
{
	char intial_c = s[0];
	string ans = "";
	string orignalpath = string(root);
	if (intial_c == '~')
	{
		ans = orignalpath + s.substr(1, s.length());
	}
	else if (intial_c == '.')
	{
		ans = string(current_Path) + s.substr(1, s.length());
	}
	else if (intial_c == '/')
	{
		ans = orignalpath + s;
	}
	else
	{

		ans = string(current_Path) + "/" + s;
	}

	return ans;
}

void clear_screen()
{
	int lastLine = row_size + 1;
	printf("%c[%d;%dH", 27, lastLine, 1);
	printf("%c[2K", 27);
	cout << ":";
}

void copy_directory(char *o_path, char *d_path)
{
	int s = mkdir(d_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (s != -1)
	{
		DIR *directory;
		struct dirent *d;
		directory = opendir(o_path);
		if (directory)
		{
			while ((d = readdir(directory)) != NULL)
			{
				if ((string(d->d_name) != "..") && (string(d->d_name) != "."))
				{
					string tmppath = string(o_path) + "/";
					tmppath += string(d->d_name);
					char *newpath = new char[tmppath.length() + 1];
					strcpy(newpath, tmppath.c_str());

					struct stat s;

					string tmp = string(d_path) + "/";
					tmp += string(d->d_name);
					char *latestdestpath = new char[tmp.length() + 1];
					strcpy(latestdestpath, tmp.c_str());

					if (stat(newpath, &s) != -1)
					{
						if (!(S_ISDIR(s.st_mode)))
						{
							copy_File(newpath, latestdestpath);
						}
						else
						{
							copy_directory(newpath, latestdestpath);
						}
					}
					else
					{
						perror("lstat");
					}
				}
			}
		}
		else
		{
			cout << endl;
			cout << "There is no directory found while copying in : " + string(o_path) << endl;
			cout << ":";
		}
	}
	else
	{
		cout << endl;
		cout << "Error occured while creating directory in : " + string(o_path) << endl;
		cout << ":";
	}
}

void copy_File(char *o_path, char *d_path)
{
	int x;
	char c[1024];
	struct stat sourcestat, deststat;
	int input;
	int output;

	input = open(o_path, O_RDONLY);
	output = open(d_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	while ((x = read(input, c, sizeof(c))) > 0)
	{
		write(output, c, x);
	}

	if (stat(d_path, &deststat) == -1)
	{
		cout << endl;
		cout << "Error occured at destination path" << endl;
		cout << ":";
	}

	if (stat(o_path, &sourcestat) == -1)
	{
		cout << endl;
		cout << "Error occured at source path" << endl;
		cout << ":";
	}

	int a = chown(d_path, sourcestat.st_uid, sourcestat.st_gid);
	if (a != 0)
	{
		cout << endl;
		cout << "Error occured while setting ownership using chown" << endl;
		cout << ":";
	}

	int b = chmod(d_path, sourcestat.st_mode);
	if (b != 0)
	{
		cout << endl;
		cout << "Error occured while setting ownership using chmod" << endl;
		cout << ":";
	}
}

void move(vector<string> v)
{
	int total = v.size();
	if (total < 3)
	{
		cout << endl;
		cout << "Move command expect more number of arguments :" << endl;
		cout << ":";
	}
	else
	{

		for (int i = 1; i < total - 1; i++)
		{
			string new_path = v[i];
			string temp;
			size_t last_p;
			last_p = new_path.find_last_of("/\\");
			temp = new_path.substr(last_p + 1, new_path.length());
			char *o_path = new char[new_path.length() + 1];
			strcpy(o_path, new_path.c_str());

			string tmp = v[total - 1];
			tmp += "/";
			tmp += temp;
			char *d_path = new char[tmp.length() + 1];
			strcpy(d_path, tmp.c_str());

			if (!is_directory(o_path))
			{
				copy_File(o_path, d_path);
				remove_onefile(o_path);
			}
			else
			{
				copy_directory(o_path, d_path);
				remove_onedir(o_path);
			}
		}
	}
}

int is_directory(char *newpath)
{
	struct stat s;
	if (stat(newpath, &s) != -1)
	{
		if ((S_ISDIR(s.st_mode)))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		perror("lstat");
	}

	return 0;
}

void copy_command(vector<string> v)
{
	int total = v.size();
	if (total < 3)
	{
		cout << endl;
		cout << "Number of arguments is invalid in copy command: " << endl;
		cout << ":";
	}
	else
	{

		for (int i = 1; i < total - 1; i++)
		{

			string new_path = v[i];
			string temp;
			size_t last_p;
			last_p = new_path.find_last_of("/\\");
			temp = new_path.substr(last_p + 1, new_path.length());
			char *o_path = new char[new_path.length() + 1];
			strcpy(o_path, new_path.c_str());

			string tmp = v[total - 1];
			tmp += "/";
			tmp += temp;
			char *d_path = new char[tmp.length() + 1];
			strcpy(d_path, tmp.c_str());

			if (!is_directory(o_path))
			{
				copy_File(o_path, d_path);
			}
			else
			{
				copy_directory(o_path, d_path);
			}
		}
	}
}

int command_mode()
{
	char c;
	do
	{
		string input;
		while (((c = getchar()) != 10) && c != 27)
		{
			if (c != 127)
			{
				input = input + c;
				cout << c;
			}
			else
			{
				int len = input.length();
				clear_screen();
				if (len > 1)
				{
					input = input.substr(0, len - 1);
				}
				else
				{
					input = "";
				}
				cout << input;
			}
		}

		int i = 0;
		input_list.clear();
		bool f = false;
		while (i < input.length())
		{
			string sub = "";
			while (input[i] != ' ' && i < input.length())
			{
				if (input[i] != '\\')
				{
					sub += input[i];
					i++;
				}
				else
				{
					sub += input[i + 1];
					i = i + 2;
				}
			}
			if (!f)
			{
				input_list.push_back(sub);
				//cout<<"sub --- "<<sub<<endl;

				if (input_list[0] != "create_file" && input_list[0] != "create_dir" && input_list[0] != "search")
				{
					f = true;
				}
			}
			else
			{
				string abspath = get_absolutepath(sub);
				input_list.push_back(abspath);
				//cout<<"abs "<<abspath<<endl;
				//cout<<"sub --- "<<abspath<<endl;
			}
			i++;
		}

		if (c == 10)
		{
			string operation = input_list[0];
			if (operation == "create_file")
			{
				create_mulfiles(input_list);
				clear_screen();
			}
			else if (operation == "delete_file")
			{
				remove_mulfiles(input_list);
				clear_screen();
			}
			else if (operation == "goto")
			{
				string ip_path;
				if (input_list.size() != 2)
				{
					cout << endl;
					cout << "Number of Arguments required by Goto command is invlid : " << endl;
					cout << ":";
				}
				else
				{
					ip_path = input_list[1];
				}
				char *dest_path = new char[ip_path.length() + 1];
				strcpy(dest_path, ip_path.c_str());
				backward_stack.push(string(current_Path));
				while (!forward_stack.empty())
				{
					forward_stack.pop();
				}
				current_Path = dest_path;
				return 1;
			}
			else if (operation == "create_dir")
			{
				make_Dir(input_list);
				clear_screen();
			}
			else if (operation == "delete_dir")
			{
				remove_muldir(input_list);
				clear_screen();
			}
			else if (operation == "copy")
			{

				copy_command(input_list);
				clear_screen();
			}
			else if (operation == "rename")
			{
				rename_file(input_list);
				clear_screen();
			}
			else if (operation == "move")
			{
				move(input_list);
				clear_screen();
			}

			else if (operation == "search")
			{
				int s = search_cmd(input_list);
				clear_screen();
			}

			
			else
			{
				cout << endl;
				cout << "Command is not valid" << endl;
				cout << ":";
				clear_screen();
			}
		}

	} while (c != 27);

	return 0;
}

void make_Dir(vector<string> v)
{

	int total = v.size();
	if (v.size() > 2)
	{
		string d_path = get_absolutepath(v[total - 1]);
		for (int i = 1; i < total - 1; i++)
		{
			string tmp = d_path + "/";
			tmp += v[i];
			char *final_path = new char[tmp.length() + 1];
			strcpy(final_path, tmp.c_str());
			int s = mkdir(final_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (s == -1)
			{
				cout << endl;
				cout << "Error occured in creating Directory in : " + string(tmp) << endl;
				cout << ":";
			}
		}
	}
	else
	{
		cout << endl;
		cout << "Number of Arguments are invalid in create_dir" << endl;
		cout << ":";
	}
}

void do_navigation()
{

	cor_X = 1, cor_Y = 1;
	char ch;
	struct termios normal_setting, new_setting;
	current_Path = root;
	printf("%c[%d;%dH", esc, cor_X, cor_Y);

	tcgetattr(fileno(stdin), &normal_setting);
	new_setting = normal_setting;
	new_setting.c_lflag &= ~ECHO;
	new_setting.c_lflag &= ~ICANON;

	if (tcsetattr(fileno(stdin), TCSAFLUSH, &new_setting) != 0)
	{
		cout << endl;
		cout << "Error Occured while changing mode " << endl;
	}
	else
	{

		while (1)
		{
			int ll = row_size + 1;
			printf("%c[%d;%dH", esc, ll, 1);
			cout << "<-NORMAL MODE->";
			printf("%c[%d;%dH", esc, cor_X, cor_Y);
			ch = cin.get();
			if (ch == 27)
			{
				ch = cin.get();
				ch = cin.get();

				if (ch == 'A')
				{
					if (cor_X > 1)
					{
						cor_X--;
						if (cor_X > 0)
						{
							printf("%c[%d;%dH", esc, cor_X, cor_Y);
						}
						else if (cor_X <= 0)
						{
							//cls;
							printf("%c[2J", esc);
							printf("%c[%d;%dH", esc, 1, 1);
						}
					}
				}

				else if (ch == 'B')
				{
					int lenRecord;
					if (cor_X < (total_NoofFiles))
					{
						cor_X++;
						if (cor_X <= row_size)
						{
							printf("%c[%d;%dH", esc, cor_X, cor_Y);
						}
						else if (cor_X > row_size && cor_X <= total_NoofFiles)
						{
							printf("%c[2J", esc);
							lenRecord = get_Filecount() - 1;
							printf("%c[%d;%dH", esc, 1, 1);
						}
						printf("%c[%d;%dH", esc, cor_X, cor_Y);
					}
				}
				else if (ch == 'C')
				{
					if (!forward_stack.empty())
					{
						string temp = string(current_Path);
						if (search_Flag != 1)
						{
							backward_stack.push(string(temp));
						}
						string t = forward_stack.top();
						strcpy(current_Path, t.c_str());
						forward_stack.pop();
						search_Flag = 0;
						open_Dir(current_Path);
						cor_X = 1, cor_Y = 1;
						printf("%c[%d;%dH", esc, cor_X, cor_Y);
					}
				}
				else if (ch == 'D')
				{
					if (!backward_stack.empty())
					{
						string cpath = string(current_Path);
						if (search_Flag != 1)
						{
							forward_stack.push(cpath);
						}
						string t = backward_stack.top();
						strcpy(current_Path, t.c_str());
						backward_stack.pop();
						search_Flag = 0;
						open_Dir(current_Path);
						cor_X = 1, cor_Y = 1;
						printf("%c[%d;%dH", esc, cor_X, cor_Y);
					}
				}
			}
			//HOME key pressed
			else if (ch == 104 || ch == 72)
			{
				if (string(current_Path) != string(root))
				{
					if (search_Flag != 1)
						backward_stack.push(string(current_Path));
					while (!forward_stack.empty())
					{
						forward_stack.pop();
					}
					strcpy(current_Path, root);
					search_Flag = 0;
					open_Dir(current_Path);
					cor_X = 1, cor_Y = 1;
					printf("%c[%d;%dH", esc, cor_X, cor_Y);
				}
			}
			//Back-Space key pressed
			else if (ch == 127)
			{
				if ((strcmp(current_Path, root) != 0) && search_Flag != 1)
				{
					backward_stack.push(string(current_Path));
					while (!forward_stack.empty())
					{
						forward_stack.pop();
					}
					size_t last;
					string tmp1;
					string tempPath = string(current_Path);
					last = tempPath.find_last_of("/\\");
					tmp1 = tempPath.substr(0, last);
					strcpy(current_Path, tmp1.c_str());
					open_Dir(current_Path);
					cor_X = 1, cor_Y = 1;
					printf("%c[%d;%dH", esc, cor_X, cor_Y);
				}
			}
			//Enter key pressed
			else if (ch == 10)
			{

				string current_d = d_List[cor_X - 1];
				string newpath;
				struct stat s;
				if (search_Flag != 1)
				{
					newpath = string(current_Path) + "/" + current_d;
				}
				else
				{
					newpath = current_d;
				}

				char *final_path = new char[newpath.length() + 1];
				strcpy(final_path, newpath.c_str());

				stat(final_path, &s);

				//file is Regular File
				if ((s.st_mode & S_IFMT) == S_IFREG)
				{

					pid_t c_pid;
					int c_status;
					c_pid = fork();
					pid_t temppid;
					if (c_pid == 0)
					{
						string s = "/usr/bin/vi";

						char *tmp1 = new char[s.length() + 1];
						strcpy(tmp1, s.c_str());
						char *tmp2 = new char[d_List[cor_X - 1].length() + 1];
						strcpy(tmp2, d_List[cor_X - 1].c_str());

						char *args[] = {tmp1, tmp2, NULL};
						execv(args[0], args);
						printf("Error....");
						exit(0);
					}
					else
					{
						do
						{
							temppid = wait(&c_status);
						} while (temppid != c_pid);
					}
				}
				//If file type is Directory
				else if ((s.st_mode & S_IFMT) == S_IFDIR)
				{
					//cout<<"DIR"<<endl;
					cor_X = 1;
					search_Flag = 0;
					if (current_d == string(".."))
					{
						backward_stack.push(string(current_Path));
						//stack_Clear(forward_stack);
						while (!forward_stack.empty())
						{
							forward_stack.pop();
						}
						//setBackPath(current_Path);
						size_t last_pos;
						string tmp1;
						string tempPath = string(current_Path);
						last_pos = tempPath.find_last_of("/\\");
						tmp1 = tempPath.substr(0, last_pos);
						strcpy(current_Path, tmp1.c_str());
					}
					else if (current_d == string("."))
					{
						//Stay here only
					}

					else
					{
						if (current_Path != NULL)
						{
							backward_stack.push(string(current_Path));
							//stack_Clear(forward_stack);
							while (!forward_stack.empty())
							{
								forward_stack.pop();
							}
						}
						current_Path = final_path;
					}

					open_Dir(current_Path);
				}
				else
				{
					cout << endl;
					cout << "Unable to Open directory: " << endl;
					cout << ":";
				}
			}
			// : key pressed
			else if (ch == 58)
			{
				int lastLine = row_size + 1;
				printf("%c[%d;%dH", esc, lastLine, 1);
				printf("%c[2K", 27);
				cout << ":";
				int result = command_mode();
				cor_X = 1;
				printf("%c[%d;%dH", esc, cor_X, cor_Y);
				if (result == 1)
				{
					open_Dir(current_Path);
				}
				else if (result == 2)
				{
				}
				else
				{
					search_Flag = 0;
					open_Dir(current_Path);
				}
			}
		}
	}

	tcsetattr(fileno(stdin), TCSANOW, &normal_setting);
}

int get_Filecount()
{
	struct winsize w;
	int rec_size;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	row_size = w.ws_row - 1;
	column_size = w.ws_col;
	if (total_NoofFiles > row_size)
	{
		rec_size = total_NoofFiles;
	}
	else
	{
		rec_size = row_size;
	}
	return rec_size;
}

void print_fun(const char *d_name, const char *root)
{
	string finalpath;
	struct tm dt;
	finalpath = string(root) + "/" + string(d_name);
	char *path = new char[finalpath.length() + 1];
	strcpy(path, finalpath.c_str());
	struct stat sb;
	if (stat(path, &sb) == -1)
	{
		perror("lstat");
	}

	printf("%s", d_name);
	printf("\t");
	printf("%10.2fK", ((double)sb.st_size) / 1024);
	printf("\t");
	printf((S_ISDIR(sb.st_mode)) ? "d" : "-");
	printf((sb.st_mode & S_IRUSR) ? "r" : "-");
	printf((sb.st_mode & S_IWUSR) ? "w" : "-");
	printf((sb.st_mode & S_IXUSR) ? "x" : "-");
	printf((sb.st_mode & S_IRGRP) ? "r" : "-");
	printf((sb.st_mode & S_IWGRP) ? "w" : "-");
	printf((sb.st_mode & S_IXGRP) ? "x" : "-");
	printf((sb.st_mode & S_IROTH) ? "r" : "-");
	printf((sb.st_mode & S_IWOTH) ? "w" : "-");
	printf((sb.st_mode & S_IXOTH) ? "x" : "-");

	dt = *(gmtime(&sb.st_mtime));
	printf("\t %d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900,
		   dt.tm_hour, dt.tm_min, dt.tm_sec);

	printf("\n");


}

void open_Dir(const char *path)
{
	struct dirent *d;
	DIR *directory;
	int cnt = 0;
	directory = opendir(path);
	if (!directory)
	{
		cout << endl;
		cout << "Unable to open Directory:" << endl;
		return;
	}
	d_List.clear();

	while ((d = readdir(directory)) != NULL)
	{
		if ((string(d->d_name) != "..") || (strcmp(path, root) != 0))
		{
			d_List.push_back(string(d->d_name));
			cnt++;
		}
	}
	sort(d_List.begin(), d_List.end());
	int l = get_Filecount();
	total_NoofFiles = cnt;
	printf("\033[H\033[J");
	printf("%c[%d;%dH", 27, 1, 1);

	for (int i = 0, j = 1; i < total_NoofFiles && j <= l; i++, j++)
	{
		char *tmp = new char[d_List[i].length() + 1];
		strcpy(tmp, d_List[i].c_str());
		print_fun(tmp, path);
	}

	printf("%c[%d;%dH", 27, 1, 1);
}

int main(int argc, char *argv[])
{

	string tmp = ".";
	char *tmp_path = new char[tmp.length() + 1];
	strcpy(tmp_path, tmp.c_str());
	root = tmp_path;
	open_Dir(tmp_path);
	do_navigation();

	return 0;
}
