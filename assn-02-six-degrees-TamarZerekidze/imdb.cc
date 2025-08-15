using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <string.h>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

 struct Properties {
 void* info;
 void* file;
};

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

int actorCmpFn(const void *act1, const void *act2){
  Properties* act = (Properties*)act1;
  char* name1 = *(char**)act->info;
  void* addr1 = act->file;
  char* name2 = (char*)addr1 + *(int*)act2;

  return strcmp(name1,name2);
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const {
   int actorN = *(int*)actorFile;
   const char* actorName = player.c_str(); 
   Properties actor;
   actor.info = &actorName;
   actor.file = (void*)actorFile;
   
   void* actorOffset = bsearch(&actor, (char*)actorFile +sizeof(int), actorN, sizeof(int), actorCmpFn);
   if(actorOffset == NULL) return false;
   int curr = *(int*)actorOffset;
   char* actorAddress = (char*)actorFile + curr;
   
   char* ptr = actorAddress;
   curr = 0;
   // moving right in memory to take out movies of this actor
   curr += player.size() + 1;
   if(player.size() %2 == 0) curr++; 
   ptr += curr;
   
   short filmN = *(short*)ptr;
   curr += sizeof(short);
   
   if(curr % 4 != 0) ptr +=(sizeof(short) + 2);
   else ptr += sizeof(short);

   //putting movies in a vector
   for (short i = 0; i < filmN; i++){
    int movieOffset = *((int*)ptr + i);
    char* flm = (char*)movieFile + movieOffset;
    film filmInfo;
    string filmTitle = flm;
    int filmYear = 1900 + *(flm + filmTitle.size() + 1);
    
    filmInfo.title = filmTitle;
    filmInfo.year = filmYear;
    films.push_back(filmInfo);
   }
   
   return true;

   }

int movieCmpFn(const void *mv1, const void *mv2){
  Properties* firstMovie = (Properties*)mv1;
  film* film1 = (film*)firstMovie->info;
  string name1 = film1->title;
  void* addr1 = firstMovie->file;

  film film2;
  char* nameAddr = (char*)addr1 + *(int*)mv2;
  string name2 = nameAddr;
  film2.title = name2;
  int year2 = 1900 + *(nameAddr + name2.size() + 1);
  film2.year = year2;

  if(*film1 == film2) return 0;
  else if (*film1 < film2) return -1;
  else return 1;

}

bool imdb::getCast(const film& movie, vector<string>& players) const {
   int movieN = *(int*)movieFile;
   Properties currMovie;
   currMovie.info = (void*) &movie;
   currMovie.file = (void*) movieFile;

   void* filmOffset = bsearch(&currMovie, (int*)movieFile + 1, movieN, sizeof(int), movieCmpFn);
   if (filmOffset == NULL) return false;

   int curr = *(int*)filmOffset;
   char* movieAddress = (char*)movieFile + curr;
   char* moviePtr = movieAddress;
   curr = 0;
   // moving right in memory to take out actors of this movie
   curr += movie.title.size() + 2;
   if (curr % 2 == 1) curr++;
   moviePtr += curr;

   short playerN = *(short*)moviePtr;
   curr += 2;
   if(curr % 4 != 0) moviePtr +=(sizeof(short) + 2);
   else moviePtr += sizeof(short);

   //putting actors in a vector
   for (short i = 0; i < playerN; i++){
    int actOffset = *((int*)moviePtr + i);
    char* actr = (char*)actorFile + actOffset;
    string actorName = actr; 
    players.push_back(actorName);
   }
   
   return true;
   }

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
