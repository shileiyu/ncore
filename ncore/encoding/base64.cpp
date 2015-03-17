#include "base64.h"

namespace ncore
{


const char Base64::kBase64[] =
{
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
};

const char Base64::kPad64 = '=';

int Base64::Encode(const void * blob,
                   size_t blob_size, 
                   char * code, 
                   size_t code_size)
{
    size_t datalength = 0;
    uint8_t input[3];
    uint8_t output[4];
    size_t i;

    const uint8_t * src = reinterpret_cast<const uint8_t *>(blob);
    
    while (2 < blob_size) {
        input[0] = *src++;
        input[1] = *src++;
        input[2] = *src++;
        blob_size -= 3;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
        output[3] = input[2] & 0x3f;
        assert(output[0] < 64);
        assert(output[1] < 64);
        assert(output[2] < 64);
        assert(output[3] < 64);

        if (datalength + 4 > code_size)
            return (-1);
        code[datalength++] = kBase64[output[0]];
        code[datalength++] = kBase64[output[1]];
        code[datalength++] = kBase64[output[2]];
        code[datalength++] = kBase64[output[3]];
    }

    /* Now we worry about padding. */
    if (0 != blob_size) {
        /* Get what's left. */
        input[0] = input[1] = input[2] = '\0';
        for (i = 0; i < blob_size; i++)
            input[i] = *src++;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
        assert(output[0] < 64);
        assert(output[1] < 64);
        assert(output[2] < 64);

        if (datalength + 4 > code_size)
            return (-1);
        code[datalength++] = kBase64[output[0]];
        code[datalength++] = kBase64[output[1]];
        if (blob_size == 1)
            code[datalength++] = kPad64;
        else
            code[datalength++] = kBase64[output[2]];
        code[datalength++] = kPad64;
    }
    if (datalength >= code_size)
        return (-1);
    code[datalength] = '\0';	/* Returned value doesn't count \0. */
    return (datalength);
}


int Base64::Decode(const char * code,
                   void * blob, 
                   size_t blob_size)
{
    size_t tarindex;
    int state, ch;
    const char *pos;
    uint8_t * target = reinterpret_cast<uint8_t *>(blob);

    state = 0;
    tarindex = 0;

    while ( (ch = *code++) != '\0') 
    {
        if (isspace(ch))	/* Skip whitespace anywhere. */
            continue;

        if (ch == kPad64)
            break;

        pos = strchr(kBase64, ch);
        if (pos == 0) 		/* A non-base64 character. */
            return (-1);

        switch (state) {
        case 0:
            if (target) {
                if (tarindex >= blob_size)
                    return (-1);
                target[tarindex] = (pos - kBase64) << 2;
            }
            state = 1;
            break;
        case 1:
            if (target) {
                if (tarindex + 1 >= blob_size)
                    return (-1);
                target[tarindex]   |=  (pos - kBase64) >> 4;
                target[tarindex+1]  = ((pos - kBase64) & 0x0f)
                    << 4 ;
            }
            tarindex++;
            state = 2;
            break;
        case 2:
            if (target) {
                if (tarindex + 1 >= blob_size)
                    return (-1);
                target[tarindex]   |=  (pos - kBase64) >> 2;
                target[tarindex+1]  = ((pos - kBase64) & 0x03)
                    << 6;
            }
            tarindex++;
            state = 3;
            break;
        case 3:
            if (target) {
                if (tarindex >= blob_size)
                    return (-1);
                target[tarindex] |= (pos - kBase64);
            }
            tarindex++;
            state = 0;
            break;
        default:
            assert(0);
        }
    }

    /*
    * We are done decoding Base-64 chars.  Let's see if we ended
    * on a byte boundary, and/or with erroneous trailing characters.
    */

    if (ch == kPad64) 
    {
        /* We got a pad char. */
        /* Skip it, get next. */
        ch = *code++;	
        switch (state) 
        {
        case 0:		/* Invalid = in first position */
        case 1:		/* Invalid = in second position */
            return (-1);
        case 2:		/* Valid, means one byte of info */
            /* Skip any number of spaces. */
            for (; ch != '\0'; ch = *code++)
                if (!isspace(ch))
                    break;
            /* Make sure there is another trailing = sign. */
            if (ch != kPad64)
                return (-1);

            ch = *code++;		/* Skip the = */
            /* Fall through to "single trailing =" case. */
            /* FALLTHROUGH */
        case 3:		/* Valid, means two bytes of info */
            /*
            * We know this char is an =.  Is there anything but
            * whitespace after it?
            */
            for (; ch != '\0'; ch = *code++)
                if (!isspace(ch))
                    return (-1);
            /*
            * Now make sure for cases 2 and 3 that the "extra"
            * bits that slopped past the last full byte were
            * zeros.  If we don't check them, they become a
            * subliminal channel.
            */
            if (target && target[tarindex] != 0)
                return (-1);
        }
    } 
    else 
    {
        /*
        * We ended by seeing the end of the string.  Make sure we
        * have no partial bytes lying around.
        */
        if (state != 0)
            return (-1);
        if (tarindex >= blob_size)
            return (-1);

        target[tarindex] = 0;
    }

    return (tarindex);
}


}
