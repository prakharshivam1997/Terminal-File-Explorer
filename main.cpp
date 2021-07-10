#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<iostream>
#include<bits/stdc++.h>
#include<string.h>
#include<string>
#include<stdlib.h>
#include<pwd.h>
#include<stdio.h>
#include <ctime>
#include <fcntl.h>
#include <sys/sendfile.h> 
#include<stdlib.h>
#include <grp.h>  
std::vector<dirent*>file;
std::vector<std::string>backstack;
std::vector<std::string>forwardstack;
struct winsize w;
int startLine=0;
int endLine=0;
int window_size;
int cursor;
std::string rootPath;
std::string currentDir;
bool flag=false;
int topIdx;
int bottomIdx;
std::vector<std::string>vst;
std::string cModeInput;
bool searchFlag=false;
int findMin(int x, int y)
{
    if(x<y)
    {
        return x;
    }
    else
    {
        return y;
    }
    
}
bool ifDirectory(std::string path)
{
    struct stat sbuf;
    if( stat(path.c_str(),&sbuf) == 0 )
    {
        if( sbuf.st_mode & S_IFDIR )
        {
            return true;
        }
        else
        {
            return false;
        }
        
    }
    else
    {
        return false;
    }
}
void display(char * dirName)
{
    struct stat sb;
    std::string dirPath=currentDir+"/"+dirName;
    stat(dirPath.c_str(), &sb);
    printf((sb.st_mode & S_IRUSR) ? "r" : "-");
    printf((sb.st_mode & S_IWUSR) ? "w" : "-");
    printf((sb.st_mode & S_IXUSR) ? "x" : "-");
    printf((sb.st_mode & S_IRGRP) ? "r" : "-");
    printf((sb.st_mode & S_IWGRP) ? "w" : "-");
    printf((sb.st_mode & S_IXGRP) ? "x" : "-");
    printf((sb.st_mode & S_IROTH) ? "r" : "-");
    printf((sb.st_mode & S_IWOTH) ? "w" : "-");
    printf((sb.st_mode & S_IXOTH) ? "x" : "-");
     struct passwd* get_username;
    get_username = getpwuid(sb.st_uid);
    std::string uname = get_username->pw_name;
    printf(" %10s ", uname.c_str());

    struct group* get_grpname;
    get_grpname = getgrgid(sb.st_gid);
    std::string gname = get_grpname->gr_name;
    printf(" %10s ", gname.c_str());
    std::string m_time = std::string(ctime(&sb.st_mtime));
    m_time = m_time.substr(4, 12);
    printf(" %-12s ", m_time.c_str());

}  
void printFile()
{
    printf("\033c");
    for(int i=topIdx;i<=bottomIdx;i++)
    {
        if(i==bottomIdx)
        {
            display(file[i]->d_name);
            std::cout<<file[i]->d_name;
            continue;
        }
        else
        {
            char temp[1000];
            strcpy(temp,file[i]->d_name);
            display(file[i]->d_name);
            //if(temp==NULL)
            std::cout<<temp<<std::endl;
        }
    }
    
}
void moveCursor(int x,int y) {
	std::cout<<"\033["<<x<<";"<<y<<"H";
	fflush(stdout);
}

void setRoot(std::string path)
{
    rootPath=path;
}
void windowSize()
{
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
    window_size=w.ws_row;
}
void setDirectory(std::string dir)
{
    DIR* dp;
	struct dirent* entry;
    currentDir=dir;
	if((dp=opendir(dir.c_str()))==NULL){
		fprintf(stderr, "Can't open the Directory!\n");
		return;
	}
	chdir(dir.c_str());
	file.clear();
	while((entry=readdir(dp))!=NULL){
        if(ifDirectory(entry->d_name))
        {
            //if((strcmp(".",entry->d_name)!=0)&&(strcmp("..",entry->d_name)!=0))
            {
                //if(entry->d_name[0]!='.')
                {
                    file.push_back(entry);
                }
            }
        }
        else
        {
            //backstack.push_back(currentDir+"/"+entry->d_name);
            file.push_back(entry);
        }
        
	}
	closedir(dp);
    windowSize();
    if(file.size()>=window_size)
    {
        endLine=window_size;
    }
    else
    {
        endLine=(file.size())%(window_size);
    }
    cursor=endLine;
    topIdx=0;
    bottomIdx=file.size()-1;
    startLine=1;
	printFile();
    moveCursor(endLine,0);
    bottomIdx=file.size()-1;
    topIdx=file.size()-(endLine);
	return;
}
void openFiles(std::string pathToTake)
{
    if (ifDirectory(pathToTake))
    {
        backstack.push_back(currentDir);
        setDirectory(pathToTake);
    }
    else
    {
        pid_t pid=fork();
		if(pid==0)
        {
            std::string temp=(pathToTake);
			execlp("gedit","gedit",temp.c_str(),(char*)NULL);
			exit(1);
		}
    }
}
void goBack()
{
    if(backstack.empty()==true)
    {
        return;
    }
    else
    {
        std::string strPath=backstack.back();
        backstack.pop_back();
        forwardstack.push_back(currentDir);
        setDirectory(strPath);
    }
}
void goForward()
{
    if(forwardstack.empty()==true)
    {
        return;
    }
    else
    {
        std::string strPath=forwardstack.back();
        forwardstack.pop_back();
        backstack.push_back(currentDir);
        setDirectory(strPath);
    }
    
}
std::string pathCreate(std::string tstr)
{
    //std::cout<<tstr<<std::endl;
    if(tstr[0]=='/')
    {
        return tstr;
    }
    else if(tstr[0]=='~')
    {
        //std::cout<<rootPath+'/'+tstr.substr(2,tstr.size()-1)<<std::endl;
        return rootPath+'/'+tstr.substr(2,tstr.size()-1);
    }
    else
    {
        return "incorrect";
    }
}
void createDirectory(std::string dirName,std::string dirPath)
{
    std::string fpath=dirPath+"/"+dirName;
    mkdir(fpath.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    std::cout<<dirName<<"created successfully"<<std::endl;
}
void createFile(std::string fileName, std::string filePath)
{
   std::fstream file; 
   file.open((filePath+'/'+fileName).c_str(),std::ios::out); 
   if(!file) 
   { 
       std::cout<<"Error in creating file!!!"; 
   } 
   file.close(); 
  
}
void copyFile(std::string sourcePath,std::string destPath)
{
    //std::cout<<sourcePath<<std::endl;
	int source = open(sourcePath.c_str(), O_RDONLY, 0);
    int dest = open(destPath.c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);
    struct stat stat_source;
    fstat(source, &stat_source);

    sendfile(dest, source, 0, stat_source.st_size);

    close(source);
    close(dest);
	return;
}
std::string nameExtractor(std::string str)
{
    std::string temp;
    for(int i=str.size()-1;i>=0;i--)
    {
        if(str[i]=='/')
        {
            break;
        }
        else
        {
            temp.push_back(str[i]);
        }
    }
    reverse(temp.begin(),temp.end());
    return temp;
}
void copyDir(std::string dir,std::string dest){
	DIR *dp;
	struct dirent *e;
	struct stat statbuf;

	if((dp = opendir(dir.c_str()))==NULL)
    {
		std::cout<<"Cannot open";
		return;
	}
	chdir(dir.c_str());
	while((e= readdir(dp))!=NULL)
    {
		lstat(e->d_name,&statbuf);
		if(ifDirectory(dir +'/'+e->d_name ))
        {
			if(strcmp(".",e->d_name)==0 || strcmp("..",e->d_name)==0)
            {
				continue;
			}
			mkdir((dest+'/'+e->d_name).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
			copyDir(dir+'/'+e->d_name,dest+'/'+e->d_name);
		}
		else{
			copyFile(dir+'/'+e->d_name,dest+'/'+e->d_name);
		}
	}
	chdir("..");
	closedir(dp);
	return;
}
void renameFile(std::string sourcePath,std::string destPath)
{
    int value=rename(sourcePath.c_str(), destPath.c_str());
    if(value==0)
    {
        std::cout<<"done with renaming"<<std::endl;
    }
    else
    {
        std::cout<<"error in renaming"<<std::endl;
    }    
}
void deleteDir(std::string dir){
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;

	if((dp = opendir(dir.c_str()))==NULL){
		fprintf(stderr, "Can't open the directory: %s\n",dir.c_str());
		return;
	}
	chdir(dir.c_str()); 
	while((entry = readdir(dp))!=NULL){
		lstat(entry->d_name,&statbuf);
		if(S_ISDIR(statbuf.st_mode)){
			if(strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0){
				continue;
			}
			deleteDir(entry->d_name);
			rmdir(entry->d_name);
		}
		else{
			unlink(entry->d_name);
		}
	}
	chdir("..");
	closedir(dp);
}
void moveDir(std::string sourcePath,std::string destPath)
{
    if(ifDirectory(sourcePath)==false)
    {
        std::cout<<sourcePath<<std::endl;
        std::string dir_name=nameExtractor(sourcePath);
        copyFile(sourcePath,destPath+'/'+dir_name);
        unlink(sourcePath.c_str());
    }
    else
    {
        std::string dir_name=nameExtractor(sourcePath);
        std::cout<<dir_name<<std::endl;
        mkdir(((destPath)+'/'+dir_name).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
        copyDir(sourcePath,destPath+'/'+dir_name);
        deleteDir(sourcePath);
        rmdir(sourcePath.c_str());
    }
    
}
void searchFiles(std::string fileName,std::string dir){
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;

	if((dp = opendir(dir.c_str()))==NULL){
		fprintf(stderr, "Can't open the directory: %s\n",dir.c_str());
		return;
	}
	chdir(dir.c_str()); 
	while((entry = readdir(dp))!=NULL)
    {
		lstat(entry->d_name,&statbuf);
		if(S_ISDIR(statbuf.st_mode)){
			if(strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0){
				continue;
			}
            else if(strcmp(fileName.c_str(),entry->d_name)==0)
            {
                searchFlag=true;
                break;
            }
            else
			{
                char buf[1000];
                getcwd(buf,1000);
                searchFiles(fileName,entry->d_name);
            }
		}
		else
        {
			if(strcmp(fileName.c_str(),entry->d_name)==0)
            {
                searchFlag=true;
			}
		}
	}
	chdir("..");
	closedir(dp);
}
void actionDoer()
{
    if(vst[0]=="create_dir")
    {
        std::string dsPath=pathCreate(vst[2]);
        std::cout<<dsPath<<std::endl;
        createDirectory(vst[1],dsPath);
        chdir(currentDir.c_str());
        setDirectory(currentDir);
        printf("\033[1B");
        std::cout<<std::endl;
        printf("\033[100D");
        std::cout<<"directory created successfully"<<std::endl;
    }
    if(vst[0]=="create_file")
    {
        std::string dstPath=pathCreate(vst[2]);
        createFile(vst[1],dstPath);
        chdir(currentDir.c_str());
        setDirectory(currentDir);
        printf("\033[1B");
        std::cout<<std::endl;
        printf("\033[100D");
        std::cout<<"file created successfully"<<std::endl;
    }
    if(vst[0]=="copy")
    {
        std::string destPath=pathCreate(vst[vst.size()-1]);
        for(int i=1;i<vst.size()-1;i++)
        {
            std::string srcPath=pathCreate(vst[i]);
            if(ifDirectory(srcPath)==false)
            {
                std::string dir_name=nameExtractor(srcPath);
                copyFile(srcPath,destPath+'/'+dir_name);
            }
            else
            {
                std::string dir_name=nameExtractor(srcPath);
                //std::cout<<dir_name<<" "<<srcPath<<" "<<destPath<<std::endl;
                mkdir(((destPath)+'/'+dir_name).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
                copyDir(srcPath,destPath+'/'+dir_name);
            }
            chdir(currentDir.c_str());
            setDirectory(currentDir);
            printf("\033[1B");
            std::cout<<std::endl;
            printf("\033[100D");
            std::cout<<"copied successfully"<<std::endl;
        }
    }
    if(vst[0]=="rename")
    {
        std::string dstPath=pathCreate(vst[2]);
        std::string srcPath=pathCreate(vst[1]);
        renameFile(srcPath,dstPath);
        setDirectory(currentDir);
        printf("\033[1B");
        std::cout<<std::endl;
        printf("\033[100D");
        std::cout<<"renaming done successfully"<<std::endl;
    }
    if(vst[0]=="delete")
    {
        std::string dstPath=pathCreate(vst[1]);
        if(ifDirectory(dstPath)==true)
        {
            deleteDir(dstPath);
            rmdir(dstPath.c_str());
        }
        else
        {
            unlink(dstPath.c_str());
        }
        chdir(currentDir.c_str());
        setDirectory(currentDir);
        printf("\033[1B");
        std::cout<<std::endl;
        printf("\033[100D");
        std::cout<<"deletion successfully"<<std::endl;
    }
    if(vst[0]=="move")
    {
        std::string destPath=pathCreate(vst[vst.size()-1]);
        for(int i=1;i<vst.size()-1;i++)
        {
            std::string srcPath=pathCreate(vst[i]);
            moveDir(srcPath,destPath);
            chdir(currentDir.c_str());
            setDirectory(currentDir);
            printf("\033[1B");
            std::cout<<std::endl;
            printf("\033[100D");
            std::cout<<"moved successfully"<<std::endl;
        }
    }
    if(vst[0]=="search")
    {
        searchFlag=false;
        searchFiles(vst[1],rootPath);
        chdir(currentDir.c_str());
        setDirectory(currentDir);
        printf("\033[1B");
        std::cout<<std::endl;
        printf("\033[100D");
        if(searchFlag==1)
        {
            std::cout<<"Found"<<std::endl;
        }
        else
        {
            std::cout<<"Not Found"<<std::endl;
        }   
    }
    if(vst[0]=="goto")
    {
        std::string dstPath=pathCreate(vst[1]);
        backstack.push_back(currentDir);
        currentDir=dstPath;
        chdir(currentDir.c_str());
        setDirectory(currentDir);
        printf("\033[1B");
        std::cout<<std::endl;
        printf("\033[100D");
    }

}
void cannonMode()
{
    unsigned char c;
    std::string temp;
    //printf("\033c");
    temp.clear();
    printf("\033[1B");
    do
    {
        c=std::cin.get();
        if(c==127)
        {
            std::cout<<"\033[1D";
            std::cout<<" ";
            std::cout<<"\033[1D";
            if(temp.empty()==false)
            {
                temp.pop_back();
            }
            continue;
        }
        else if(c==10)
        {
            printf("\033[1B");
            printf("\033[100D");
            vst.push_back(temp);
            temp.clear();
            actionDoer();
            temp.clear();
            vst.clear();
        }
        else if(c==' ')
        {
            std::cout<<" ";
            vst.push_back(temp);
            temp.clear();
        }
        else
        {
            printf("%c",c);
            temp.push_back(c);
        }
        
    } while (c!='\033');
}
int main() 
{
    std::string path="/home";
    setRoot(path);
    setDirectory(path);
    currentDir=path;
    struct termios old_tio, new_tio;
    unsigned char c;
    tcgetattr(STDIN_FILENO,&old_tio);
    new_tio=old_tio;
    new_tio.c_lflag &=(~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);
    do 
    {
        c=getchar();
        if((c==65)||(c=='k'))
        {
            cursor--;
            if(cursor<startLine)
            {
                cursor=1;
                topIdx--;
                if(topIdx<0)
                {
                    topIdx=0;
                }
                else if((bottomIdx-topIdx+1)>window_size)
                {
                    bottomIdx--;
                }
                else if((bottomIdx-topIdx+1)==window_size)
                {
                    ;
                }
            }
            printFile();
            moveCursor(cursor,0);
        }
        else if((c==66)||(c=='l'))
        {
            cursor++;
            if(cursor>endLine)
            {
                bottomIdx++;
                if(bottomIdx>=file.size())
                {
                    cursor=endLine;
                    bottomIdx=file.size()-1;
                }
                else if((bottomIdx-topIdx+1)>window_size)
                {
                    endLine=window_size;
                    cursor=endLine;
                    topIdx++;
                }
                else if((bottomIdx-topIdx+1)==window_size)
                {
                    endLine=window_size;
                    cursor=endLine;
                }
                else if(endLine<window_size)
                {
                    endLine++;
                }
            
            }
            printFile();
            moveCursor(cursor,0);
        }
        else if(c==10)
        {
            openFiles(currentDir+"/"+file[cursor+topIdx-1]->d_name);
        }
        else if(c==68)
        {
            goBack();
        }
        else if(c==67)
        {
            goForward();
        }
        else if(c=='h')
        {
            setDirectory(rootPath);
        }
        else if(c==127)
        {
            setDirectory(currentDir+"/"+"..");
        }
        else if(c==':')
        {
            vst.clear();
            setDirectory(currentDir);
            std::cout<<std::endl;
            cannonMode();
            vst.clear();
            setDirectory(currentDir);
        }  
    } while(c!='q');
    printf("\033[H\033[J");
    tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);
    return 0;
}