// 20 bytes

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

void help() 
{
    std::cout << "usage:  cipher  [-e input_file output_file key(dir(r/l)-shift-initialization_vector)]" << "\n\t\t"
              << "[-d input_file output_file key(dir(r/l)-shift-initialization_vector)]" << "\n\t\t" 
              << "[-h]" <<'\n';
}

struct block
{
    uint8_t arr[20] {};
};

//XOR
void XOR(block& b)
{
    uint8_t gamma [20];

    for (int k = 0; k < 20; ++k)
    {
        gamma[k] = (rand() % 127) + 128;
        b.arr[k] = b.arr[k] ^ gamma[k];
    }
}

void operator>>(block& b, int leng) 
{
    uint8_t bit = 0,
            bit2 = 0;

    bit = b.arr[0] << (8 - leng);
    b.arr[0] = b.arr[0] >> leng;

    for (int i = 1; i < 20; i++)
    {
        bit2 = b.arr[i] << (8 - leng);
        b.arr[i] = b.arr[i] >> leng;
        b.arr[i] = b.arr[i] | bit;
        bit = bit2;
    }

    b.arr[0] = b.arr[0] | bit;
}

void operator<<(block& b, int leng) 
{ 
    uint8_t bit = 0,
            bit2 = 0;;

    bit = b.arr[19] >> (8 - leng);
    b.arr[19] = b.arr[19] << leng;

    for (int k = 18; k >= 0; --k)
    {
        bit2 = b.arr[k] >> (8 - leng);
        b.arr[k] = b.arr[k] << leng;
        b.arr[k] = b.arr[k] | bit;
        bit = bit2;
    }

    b.arr[19] = b.arr[19] | bit;
}

int main (int argc, char* argv[])
{
    if (argc > 1)
    {
        std::string first_one = static_cast<std::string>(argv[1]);

        if (first_one != "bill" && first_one != "Bill")
        {
            if (argc < 5)
                help();

            else {
                std::string out_name = static_cast<std::string>(argv[3]);
                std::string key = static_cast<std::string>(argv[4]);
                std::string dir;
                int shift; 
                unsigned long long init;

                std::ifstream file (argv[2], std::ios::in | std::ios::binary | std::ios::ate);
                if (file.is_open())
                {
                    //key processing 
                    int prev = key.find('-', 0);
                    int prev2 = key.find('-', prev + 1); 
                    if (prev == -1 || prev2 == -1 )
                    {
                        std::cout << "Wrong key format\n";
                        return 1;
                    } else {
                        dir = key.substr (0, prev); //r or l
                        shift = std::stoi(key.substr(prev + 1, prev2 - prev - 1));
                        if ((dir != "r" && dir != "l") || shift < 1 || shift > 8)
                        {
                            std::cout << "Wrong key format\n";
                            return 1;
                        }
                        init = std::stoull(key.substr(prev2 - prev + 2, key.size() - prev2 - prev));
                    }

                } else {
                    std::cout << "Unable to open a file\n";
                    return 1;
                }

                //reading from the file
                unsigned int size = static_cast<int> (file.tellg());
                int min;
                (size % 20 == 0) ? (min = 20) : (min = size % 20);
                unsigned int plug_size = 20 - min;
                char* text = new char [size + plug_size];
                file.seekg (0, std::ios::beg);
                file.read (text, size);

                //breaking text into blocks
                int a = 0, 
                    b = 20,
                    l = 0;
                std::vector<block> vec((size + plug_size)/20);
                for (int i = 0; i < vec.size(); ++i)
                {
                    for (int k = a; k < b; ++k)
                    {
                        vec[i].arr[l]= text[k];
                        ++l;
                    }
                    a = b;
                    b += 20;
                    l = 0;
                }


                if (static_cast<std::string>(argv[1]) == "-e")
                {

                    //plug
                    if (plug_size != 0)
                    {
                        unsigned int length_plug;
                        plug_size < 10 ? length_plug = 1 : length_plug = 2;

                        for (int i = size; i < size + plug_size - length_plug; ++i)
                            text[i] = rand() % 62 + 65;

                        int k = 0;
                        std::string str_pl_size = std::to_string(plug_size);
                        for (int i = size + plug_size - length_plug; i < size + plug_size; ++i)
                        {
                            text[i] = str_pl_size[k];
                            ++k;
                        }

                        k = size;
                        for (int i = size % 20; i < 20; ++i)
                        {
                            vec[vec.size() - 1].arr[i] = text[k];
                            ++k;
                        }
                    }

                    srand(init);

                    //XOR
                    for (int i = 0; i < vec.size(); ++i)
                        XOR(vec[i]);

                    //Shift
                    if (dir == "r")
                        for (int i = 0; i < vec.size(); ++i)
                            vec[i] >> shift;
                    else 
                        for (int i = 0; i < vec.size(); ++i)
                            vec[i] << shift; 

                    file.close();

                    //Output 
                    l = 0;
                    std::fstream outfile (argv[3], std::ios::out | std::ios::binary);
                    if (outfile.is_open())
                    {
                        for (int i = 0; i < vec.size(); ++i)
                            for (int j = 0; j < 20; ++j)
                            {
                                text[l] = vec[i].arr[j];
                                ++l;
                            }

                        outfile.write(text, size + plug_size);

                    } else std::cout << "Unable to open output file\n";

                    outfile.close();

                    std::cout << "Encrypted text is saved in the file \"" << out_name << "\"" << std::endl;
                }

                else if (static_cast<std::string>(argv[1]) == "-d")
                {
                    srand(init);

                    if (dir == "r")
                        for (int i = 0; i < vec.size(); ++i)
                            vec[i] << shift;
                    else 
                        for (int i = 0; i < vec.size(); ++i)
                            vec[i] >> shift; 

                    for (int i = 0; i < vec.size(); ++i)
                        XOR(vec[i]);

                    file.close();


                    l = 0;
                    for (int i = 0; i < vec.size(); ++i)
                        for (int j = 0; j < 20; ++j)
                        {
                            text[l] = vec[i].arr[j];
                            ++l;
                        }

                    //Unplugging
                    std::string length; //fixed
                    if (text[l - 2] > 47 && text[l-2] < 58)
                    {
                        length[0] = text[l-2];
                        length[1] = text[l - 1];

                    } else
                        length[0] = text[l - 1];                        

                    int plug_length = std::stoi(length);

                    //Output
                    std::fstream outfile (out_name, std::ios::out | std::ios::binary);

                    if (outfile.is_open())
                    {
                        outfile.write(text, size - plug_length);
                        outfile.close();

                    } else std::cout << "Unable to open output file\n";

                    std::cout << "Decrypted text is saved in the file \"" << out_name << "\"" << std::endl;

                } else help();

                delete[] text;
            }

        } else std::cout << "Remember! Reality's an illusion, the universe is a hologram, buy gold! Byeeee!\n"; //easter egg)

    } else help ();

    return 0;
}
