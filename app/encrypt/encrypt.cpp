// Copyright (c) 2010 The WBEA Authors. All rights reserved. Use of this source
// code is governed by a BSD-style license that can be found in the LICENSE
// file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../AES.h"
#include "../aes_default_key.h"

#ifdef _WIN32
// switch off secure warning
#pragma warning(push)
#pragma warning(disable:4996) // _SCL_SECURE_NO_WARNINGS
#endif

int main(int argc, char *argv[])
{
	printf("WBEA AES encryptor 1.0\n");

	if (argc < 2)
	{
		printf("Usage: encrypt source destination [password]\n");
	}
	else
	{
		const char *fname = argv[1];
		FILE *in_file = fopen(fname, "rb");
		if (!in_file)
    {
			printf("Can't open %s\n", fname);
    }
		else
		{
			printf("Encrypting file %s\n", fname);

			unsigned char *in_buf = NULL;
			unsigned char *out_buf = NULL;
			unsigned long num_blocks = 0;

			fseek(in_file, 0, SEEK_END);
			long size = ftell(in_file);
			if (size > 0)
			{	
				fseek(in_file, 0, SEEK_SET);

				const int BLOCKLEN = 128 / 8;
				num_blocks = (size + BLOCKLEN - 1) / BLOCKLEN;
				const unsigned long size_blocks = num_blocks * BLOCKLEN;
				in_buf = (unsigned char *)malloc(size_blocks);
				out_buf = (unsigned char *)malloc(size_blocks);

				if (in_buf && out_buf)
				{
					// clear last block
					memset(in_buf + size_blocks - BLOCKLEN, 0xFF, BLOCKLEN);

					// read source file
					size_t read = fread(in_buf, 1, size, in_file);
					if (read != size)
          {
						printf("Reading source file failed!");
          }
					else
					{
						const char *key = AES_DEFAULT_KEY;
						if (argc > 3)
							key = argv[3];
						int key_length = (int)strlen(key) * 8;
            if(key_length == 128)
            {
						  printf("Using key %s\n", key); 	

						  AES aes;
						  aes.SetParameters(key_length);
						  aes.StartEncryption((unsigned char*)key);
						  aes.Encrypt(in_buf, out_buf, num_blocks);

						  FILE *out_file = fopen(argv[2], "wb");
						  if (out_file)
						  {
							  fwrite(out_buf, 1, size_blocks, out_file);
							  fclose(out_file);
						  }
            }
            else
            {
              printf("Invalid key length\n"); 
            }
					}
				}
			}
			fclose(in_file);

			if (in_buf)
				free(in_buf);
			if (out_buf)
				free(out_buf);
		}
	}

	return 0;
}

#ifdef _WIN32
#pragma warning(pop)
#endif
