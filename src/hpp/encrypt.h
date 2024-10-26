#pragma once
#include <iostream>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include <string>
/***********************************************
 * Author: chenxuan-1607772321@qq.com
 * change time:2022-04-29 20:12:36
 * description: md5 base openssl
 * example: auto result=Md5::encode(text,textLen)
 ***********************************************/
class Md5 {
private:
  Md5() = delete;
  Md5(const Md5 &) = delete;

public:
  static std::string encode(const char *text, int textLen, char *buff = NULL) {
    unsigned char temp[16] = {0};
    char buffer[33] = {0}, ch[3] = {0};
    MD5((const unsigned char *)text, textLen, temp);
    for (unsigned i = 0; i < 16; i++) {
      sprintf(ch, "%02x", temp[i]);
      strcat(buffer, ch);
    }
    std::string result = buffer;
    if (buff != NULL)
      memcpy(buff, buffer, 32);
    return result;
  }
};
/***********************************************
 * Author: chenxuan-1607772321@qq.com
 * change time:2022-04-29 20:13:18
 * description:base64 base in openssl
 * example: {auto result=Base64::encode(text,textlen);
 * result=Base64::decode(text.c_str,text.size);}
 ***********************************************/
class Base64 {
private:
  Base64() = delete;
  Base64(const Base64 &) = delete;

public:
  static const char *error;
  static std::string encode(const char *text, int len) {
    if (text == NULL || len <= 0) {
      error = "wrong input";
      return "";
    }
    unsigned char *buffer =
        (unsigned char *)malloc((len + 1) * 2 * sizeof(unsigned char));
    if (buffer == NULL) {
      error = "memory malloc wrong";
      return "";
    }
    memset(buffer, 0, (len + 1) * 2 * sizeof(unsigned char));
    EVP_EncodeBlock(buffer, (const unsigned char *)text, len);
    std::string result = (char *)buffer;
    free(buffer);
    return result;
  }
  static std::string decode(const char *text, int len) {
    if (text == NULL) {
      error = "null input";
      return "";
    }
    char *buffer = (char *)malloc(len * sizeof(char));
    if (buffer == NULL) {
      error = "memory malloc wrong";
      return "";
    }
    memset(buffer, 0, len * sizeof(char));
    int nTextLen = EVP_DecodeBlock((unsigned char *)buffer,
                                   (const unsigned char *)text, len);
    if (nTextLen == -1) {
      free(buffer);
      return "";
    }
    std::string result(buffer, nTextLen);
    free(buffer);
    return result;
  }
};
const char *Base64::error = NULL;
