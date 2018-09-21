#include "docr.h"
#include <iomanip>

struct my_error_mgr {
  struct jpeg_error_mgr pub;  /* "public" fields */
  jmp_buf setjmp_buffer;    /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


static unsigned int LevenshteinDistance(const string & s, const string & t)
{
    unsigned int cost;

    // degenerate cases
    if(s.length() == 0) return t.length();
    if(t.length() == 0) return s.length();
    if(s.length() == t.length() && s == t) return 0;

    // create two work vectors of integer distances
    std::vector<unsigned int> m(t.length() + 1);
    std::vector<unsigned int> n(t.length() + 1);

    // initialize v0 (the previous row of distances)
    // this row is A[0][i]: edit distance for an empty s
    // the distance is just the number of characters to delete from t
    for(unsigned int i = 0; i < m.size(); i++) m[i] = i;

    for (unsigned int i = 0; i < s.length(); i++)
    {
        // calculate v1 (current row distances) from the previous row v0

        // first element of v1 is A[i+1][0]
        // edit distance is delete (i+1) chars from s to match empty t
        n[0] = i + 1;

        // use formula to fill in the rest of the row
        for (unsigned int j = 0; j < t.length(); j++)
        {
            cost = (s[i] == t[j]) ? 0 : 1;
            //n[j + 1] = Minimum(n[j] + 1, m[j + 1] + 1, m[j] + cost);
            n[j + 1] = n[j] + 1;
            if((m[j + 1] + 1) < n[j + 1]) n[j + 1] = m[j + 1] + 1;
            if((m[j] + cost) < n[j + 1]) n[j + 1] = m[j] + cost;
        }

        // copy v1 (current row) to v0 (previous row) for next iteration
        for (unsigned int j = 0; j < m.size(); j++)
            m[j] = n[j];
    }

    return n[t.length()];
}

double LevenshteinDistancePercent(const string & s, const string & t)
{
    double percent = 0.0, len = 0.5 * (s.length() + t.length());
    int step = (int)LevenshteinDistance(s, t);
    if(step == 0) return percent = 100.0;
    else
        return percent = (len - step) / len;
}

DOcr::DOcr()
{
    m_mode = JPEG_FILE;
    _setKeys();
}

DOcr::~DOcr()
{
    vector<JSAMPROW>::iterator t;
    for(t = m_img_buf.begin(); t != m_img_buf.end(); t++)
        free(*t);
    //vector<JSAMPARRAY>().swap(m_img_buf);
}

int DOcr::_readJpeg(const string & str)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE * fp = NULL;

    int row_stride;

    if(m_mode == JPEG_FILE)
    {

        if ((fp = fopen(str.c_str(), "rb")) == NULL)
            return -1;
    }

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
        * We need to clean up the JPEG object, close the input file, and return.
        */
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return -3;
    }

    jpeg_create_decompress(&cinfo);

    if(m_mode == JPEG_FILE)
        jpeg_stdio_src(&cinfo, fp);
    else if(m_mode == JPEG_MEM)
        jpeg_mem_src(&cinfo, (unsigned char *)str.c_str(), str.size());
    else
        return -2;

    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;

    m_img_width = cinfo.output_width;
    m_img_height = cinfo.output_height;
    m_img_components = cinfo.output_components;

    vector<JSAMPROW>::iterator t;
    for(t = m_img_buf.begin(); t != m_img_buf.end(); t++)
        delete(*t);
    vector<JSAMPROW>().swap(m_img_buf);

    while (cinfo.output_scanline < cinfo.output_height)
    {
        JSAMPROW buffer = new JSAMPLE[row_stride];
        (void) jpeg_read_scanlines(&cinfo, &buffer, 1);
        m_img_buf.push_back(buffer);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    if(m_mode == JPEG_FILE)
        fclose(fp);
    return 1;
}


int DOcr::_writeJpeg(string & str, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * fp = NULL;
    unsigned char *pBuf;
    long unsigned int iLen = 0;
    JSAMPROW row_pointer[1];

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if(m_mode == JPEG_FILE)
    {
        if((fp = fopen(str.c_str(), "wb")) == NULL)
            return -1;
        jpeg_stdio_dest(&cinfo, fp);
    }
    else if(m_mode == JPEG_MEM)
    {
        jpeg_mem_dest(&cinfo, &pBuf, &iLen);
    }
    else
        return -2;

    cinfo.image_width = m_img_width; 	// image width and height, in pixels
    cinfo.image_height = m_img_height;
    cinfo.input_components = 3;		// # of color components per pixel
    cinfo.in_color_space = JCS_YCbCr; 	// colorspace of input image
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE); // limit to baseline-JPEG values

    jpeg_start_compress(&cinfo, TRUE);

    while(cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = m_img_buf[cinfo.next_scanline];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    if(m_mode == JPEG_FILE)
        fclose(fp);
    else if(m_mode == JPEG_MEM)
    {
        str.append((char *)pBuf, iLen);
    }

    jpeg_destroy_compress(&cinfo);
    return 1;
}


void DOcr::_findCode(const string & s)
{
    int i, index = 0;
    double per, mPer = -1;
    //cout << "相似度：" << endl;
    //std::ios_base::fmtflags old = cout.flags();
    //cout.setf(std::ios_base::fixed);
    //cout.precision(2);
    for(i = 0; i < (int)m_keys.size(); i++)
    {
        per = LevenshteinDistancePercent(s, m_keys[i]);
        //cout << m_value[i] << ": " << per << "% \t";
        //if(i % 5 == 4) cout << endl;
        if(per > mPer)
        {
            mPer = per;
            index = i;
        }
    }
    //cout.flags(old);
    m_code += m_value[index];
    //cout << "m_code: " << m_code << endl;
}

void DOcr::setMode(OcrMode mode)
{
    m_mode = mode;
}

bool DOcr::readImage(const string & str, OcrMode mode)
{
    m_mode = mode;
    if(_readJpeg(str) > 0)
        return true;
    else return false;
}


bool DOcr::writeImage(string & str, OcrMode mode)
{
    m_mode = mode;
    if(_writeJpeg(str, 100) > 0) // The quality value is expressed on the 0..100 scale
        return true;
    else return false;
}

bool DOcr::run()
{
    int word_width   = 9;  //字符宽度，像素
    int word_height  = 13; //字符高度
    int offset_x     = 7;  //x偏移
    int offset_y     = 3;  //y偏移
    int word_spacing = 4;  //字符间隔
    int x, y, h, w;
    string word;
    unsigned char * row = (unsigned char *)malloc(m_img_components * sizeof(unsigned char) * word_width);
    m_code = ""; // 初始化验证码
    for(int i = 0; i < 4; i++)
    {
        word = "";
        x = i * (word_width + word_spacing) + offset_x;
        y = offset_y;
        for(h = y; h < (offset_y + word_height); h++)
        {
            memcpy((void *)row, (const void *)(m_img_buf[h] + x * m_img_components),
            m_img_components * word_width);
            for(w = 0; w < word_width * 3; w += 3)
            {
                if(row[w] < 150) word += '1';
                else
                    word += '0';
            }
        }
        _findCode(word);
    }
    return true;
}

void DOcr::getCode(string & code)
{
    code = m_code;
    return;
}
