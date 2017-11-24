// Copyright 2017 Mobvoi Inc. All Rights Reserved.

#include <algorithm>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fstream>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include "speech_sdk.h"

static pthread_mutex_t mutex;
static pthread_cond_t cond;

static const int kBufferSize = 640;
static const int kChannelNum = 2;
static const int kSampleRate = 16000;
static const int kLineWrapLength = 100;

static volatile bool in_the_session = true;
static volatile mobvoi_recognizer_type
    recognizer_type = MOBVOI_RECOGNIZER_ONLINE_ONEBOX;

static const std::string kAppKey = "2844F2D8DF07F35DB998BDDA9D130708";
FILE *resultFile;

struct AppCmdOptions {
  mobvoi_recognizer_type recognzier_type;
  std::string record_dev;
  std::string play_dev;
  int channels;
  int sample_rate;

  AppCmdOptions()
      : recognzier_type(MOBVOI_RECOGNIZER_ONLINE_ONEBOX),
        record_dev("default"),
        play_dev("default"),
        channels(1),
        sample_rate(16000) {}
};
static AppCmdOptions g_cmd_opts = AppCmdOptions();

bool ParseCmdLine(int argc, const char* argv[], AppCmdOptions* options) {
  if (argc < 2) return false;

  std::string type(argv[1]);
  if (type == "online") {
    options->recognzier_type = MOBVOI_RECOGNIZER_ONLINE_ONEBOX;
  } else if (type == "offline") {
    options->recognzier_type = MOBVOI_RECOGNIZER_OFFLINE;
  } else if (type == "mix") {
    options->recognzier_type = MOBVOI_RECOGNIZER_MIX;
  } else {
    std::cerr << "Unsupported recognizer type:" << type << std::endl;
    return false;
  }
  return true;
}

void* send_audio_thread(void* arg) {
  std::cout << "enter thread" <<  std::endl;
  std::ifstream& file = *(std::ifstream*) arg;
  const int kBatchSize = 320;
  int pos = 0;
  file.seekg(0, file.end);
  int length = file.tellg() / 2;
  file.seekg(0, file.beg);
  // char buffer[kBatchSize];
  short in_shorts[kBatchSize];

  usleep(200 * 1000);
  if (file.is_open()) {
    // std::cout << ">>>>>>>>>>>>>>>>> file length: " << &length << std::endl;
    while (pos < length) {
      // std::cout << ">>>>>>>>>>>>>>>>> pos: " <<  pos << std::endl;
      int stride =
          (pos + kBatchSize < length) ? kBatchSize : (length - pos);
      file.read((char*) &in_shorts, stride * 2);
      mobvoi_send_speech_frame((char*) &in_shorts, stride * 2);
      // std::cout << ">>>>>>>>>>>>>>>>> send frame finished!" <<  std::endl;
      pos += stride;
    }
  } else {
    std::cout << "File could not be opened!" << std::endl;
  }
  file.close();
  std::cout << ">>>>>>>>>>>>>>>>> closed. " <<  pos << std::endl;
  mobvoi_recognizer_stop();
  return NULL;
}

/*void MonoToStereo(const char* in, int in_len, const char* out, int out_len) {
  assert(in_len * 2 == out_len);
  for (int i = 0; i < in_len / 2; i++) {
    *((short*) out + 2 * i) = *((short*) in + i);
    *((short*) out + 2 * i + 1) = *((short*) in + i);
  }
}*/

/*std::string GetTTS(const std::string& final_transcription) {
  static const std::string kPromptWord = "\"prompt\":\"";
  size_t pos = final_transcription.find(kPromptWord, 0);

  if (pos != std::string::npos) {
    size_t start = pos + kPromptWord.size();
    size_t end = final_transcription.find('"', start);
    if (end != std::string::npos) {
      std::string promt_content =
          final_transcription.substr(start, end - start);
      return promt_content;
    }
  }
  return "问问没有听清,您能再说一次吗";
}*/

/*void PlaySound(const char* tts_url) {
  static const int kPlaybackBufSize = 1600;
  static const int kSampleRate = 16000;
  static const int kChannelNum = 2;

  std::string down_bytes;
  if (!HttpUtil::GetInMemory(tts_url, &down_bytes)) return;

  Player::Open(kSampleRate, kChannelNum);
  int read_pos = 0;
  char buf[kPlaybackBufSize] = {0};
  char stereoBuf[kPlaybackBufSize * 2] = {0};

  while (true) {
    int left_size = down_bytes.size() - read_pos;
    if (left_size <= 0) break;
    int actual_read = std::min(left_size, kPlaybackBufSize);
    memcpy(buf, down_bytes.c_str() + read_pos, actual_read);
    MonoToStereo(buf, actual_read, stereoBuf, actual_read * 2);
    Player::PlayAudioData(stereoBuf, actual_read * 2);
    read_pos += actual_read;
  }
  Player::Close();
}*/

/*static void* tts_read_thread(void* arg) {
  int bytes = 0;
  Player::Open(kSampleRate, kChannelNum);
  char buffer[kBufferSize];
  char stereoBuf[kBufferSize * 2];
  while ((bytes = mobvoi_tts_read_data(buffer, sizeof(buffer))) != -1) {
    MonoToStereo(buffer, bytes, stereoBuf, bytes * 2);
    Player::PlayAudioData(stereoBuf, bytes * 2);
  }
  Player::Close();
}*/

void on_remote_silence_detected() {
  std::cout << "--------> dummy on_remote_silence_detected" << std::endl;
}

void on_partial_transcription(const char* result) {
  std::cout << "--------> dummy on_partial_transcription: " << result
            << std::endl;
}

void on_final_transcription(const char* result) {
  std::cout << "--------> dummy on_final_transcription: " << result
            << std::endl;
}

void on_result(const char* result) {
  std::cout << "--------> dummy on_result: " << result << std::endl;
  if (result != NULL && !(std::string(result).empty())) {
    std::cout << "-----------------------------------------> result len: " << sizeof(result) << std::endl;
    fprintf(resultFile,"%s\n",result);
  } else {
    fprintf(resultFile,"\n");
  }
  
  std::string s(result);
  /*for (int i = 0; i < s.size(); i += kLineWrapLength) {
    int len = std::min((int) (s.size() - i), kLineWrapLength);
    std::string temp = s.substr(i, len);
    std::cout << temp << std::endl;
  }
  std::string tts = GetTTS(result);
  if (!tts.empty()) {
    pthread_t tid;
    std::cout << "tts = " << tts << std::endl;
    std::cout << "--------> play the tts sound..." << std::endl;
    mobvoi_tts_start_synthesis(MOBVOI_TTS_OFFLINE, tts.c_str());
    int result = pthread_create(&tid, NULL, tts_read_thread, NULL);
    assert(result);
  }*/

  /*pthread_mutex_lock(&mutex);
  in_the_session = false;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);*/
  in_the_session = false;  
   pthread_cond_signal(&cond);  
}

void on_error(int error_code) {
  std::cout << "--------> dummy on_error with error code: " << error_code
            << std::endl;
   in_the_session = false;
   fprintf(resultFile,"%d\n",error_code);
   pthread_cond_signal(&cond);
  /*pthread_mutex_lock(&mutex);
  in_the_session = false;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);*/
}

void on_local_silence_detected() {
  std::cout << "--------> dummy on_local_silence_detected" << std::endl;
  mobvoi_recognizer_stop();
}

void on_hotword_detected() {
  std::cout << "--------> dummy on_hotword_detected" << std::endl;
  mobvoi_hotword_stop();
  mobvoi_recognizer_start(recognizer_type);
}

void ShowUsage() {
  std::cerr << "Usage: robot [online/offline/mix]" << std::endl;
}

void on_speech_spl_generated(float spl) {
  // the sound press level is here, do whatever you want.
  // std::cout << "--------> dummy on_speech_spl_generated: spl = "
  //           << std::fixed << std::setprecision(6) << spl
  //           << std::endl;
}

int main(int argc, const char* argv[]) {
  if (argc != 3) {
    ShowUsage();
    return 1;
  }

  std::string type(argv[1]);
  if (type == "online") {
    recognizer_type = MOBVOI_RECOGNIZER_ONLINE_ONEBOX;
  } else if (type == "offline") {
    recognizer_type = MOBVOI_RECOGNIZER_OFFLINE;
  } else if (type == "mix") {
    recognizer_type = MOBVOI_RECOGNIZER_MIX;
  } else {
    ShowUsage();
    return 1;
  }
  resultFile = fopen("result","a+");
  // SDK initilalize including callback functions
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
  mobvoi_recognizer_set_params("mobvoi_folder", "./.mobvoi/");
  
  mobvoi_sdk_init(kAppKey.c_str());
  mobvoi_tts_init();
  mobvoi_recognizer_set_params(
      "location", "中国,北京市,北京市,海淀区,苏州街,3号,39.989602,116.316568");

  mobvoi_recognizer_handler_vtable* speech_handlers =
      new mobvoi_recognizer_handler_vtable;
  assert(speech_handlers != NULL);
  speech_handlers->on_error = &on_error;
  speech_handlers->on_final_transcription = &on_final_transcription;
  speech_handlers->on_partial_transcription = &on_partial_transcription;
  speech_handlers->on_local_silence_detected = &on_local_silence_detected;
  speech_handlers->on_remote_silence_detected = &on_remote_silence_detected;
  speech_handlers->on_result = &on_result;
  speech_handlers->on_volume = &on_speech_spl_generated;
  mobvoi_recognizer_set_handler(speech_handlers);

  mobvoi_hotword_handler_vtable* hotword_handler =
      new mobvoi_hotword_handler_vtable;
  hotword_handler->on_hotword_detected = &on_hotword_detected;
  mobvoi_hotword_add_handler(hotword_handler);

  if (recognizer_type == MOBVOI_RECOGNIZER_OFFLINE
      || recognizer_type == MOBVOI_RECOGNIZER_MIX) {
    mobvoi_recognizer_init_offline();
    const char* contacts[3];
    contacts[0] = "焦莹璀";
    contacts[1] = "问众智能";
    contacts[2] = "王斌";
    mobvoi_recognizer_set_data(MOBVOI_RECOGNIZER_OFFLINE_DATA_CONTACTS,
                               3,
                               contacts);

    const char* apps[3];
    apps[0] = "焦莹璀";
    apps[1] = "问众智能";
    apps[2] = "王斌";
    mobvoi_recognizer_set_data(MOBVOI_RECOGNIZER_OFFLINE_DATA_APPS, 3, apps);

    // const char* commands[3];
    // commands[0] = "焦莹璀";
    // commands[1] = "问众智能";
    // commands[2] = "王斌";
    // mobvoi_recognizer_set_data(MOBVOI_RECOGNIZER_OFFLINE_DATA_COMMANDS,
    //                            3,
    //                            commands);
    mobvoi_recognizer_build_data();
  }
  pthread_mutex_unlock(&mutex);

   // mobvoi_hotword_start();
   // pthread_mutex_lock(&mutex);
   // in_the_session = true;
   // while (in_the_session) {
   //   pthread_cond_wait(&cond, &mutex);
   // }
   // pthread_mutex_unlock(&mutex);
  std::ifstream test_file;
  test_file.open(argv[2]);
  // Read the audio file specified by the command line argument
  if (!test_file.is_open()) {
    std::cout << "Failed to open file " << argv[2] << std::endl;
    return 2;
  }
  fprintf(resultFile,"%s ",argv[2]);
  mobvoi_recognizer_start(recognizer_type);
  pthread_t tid;
  pthread_create(&tid, NULL, send_audio_thread, &test_file);
  in_the_session = true;
  pthread_mutex_lock(&mutex);
  while (in_the_session) {
    pthread_cond_wait(&cond, &mutex);
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>> waitting" << std::endl;
  }
  pthread_mutex_unlock(&mutex);
  fclose(resultFile);
  // SDK Clean up
  std::cout << "start sdk cleanup" << std::endl;
  mobvoi_sdk_cleanup();
  std::cout << "end sdk cleanup" << std::endl;
  delete speech_handlers;
  std::cout << "end dummy sender" << std::endl;
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
  return 0;
}
