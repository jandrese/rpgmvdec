# rpgmvdec
RPGMVP Decryption utility

Decrypts RPGMV files when supplied with the proper key.  

The key is stored in the System.json file, and you can extract it with:

json_pp < System.json | grep encryptionKey

To run the program, simply give it the key and the list of files.  If they have the rpgmvp extension the output files will have
the same name but with the rpgmvp extension replaced with png.  If they don't then the .png will be appended to the end.

Example:

rpgmvdec -k 00112233445566778899aabbccddeeff file1.rpgmvp file2.rpgmvp

A minimal amount of checking is done to make sure the decryption is working, but errors are still possible.
