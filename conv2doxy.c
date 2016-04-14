/*!
 * \file conv2doxy.c
 *
 * A tool to convert Leptonica function's comments to doxygen style.
 * Hacked up by Jürgen Buchmüller <pullmoll@t-online.de>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFLEN	512	/*!< maximum length of source lines */
char line[BUFFLEN];	/*!< current line buffer */
char next[BUFFLEN];	/*!< next line buffer */

/*!
 * \brief remove a number of characters from a string buffer
 * \param base base of the buffer (line or next)
 * \param src ptr to the position where to remove
 * \param n number of characters to remove
 */
static void str_remove(const char* base, char* src, size_t n)
{
	memmove(src, src + n, BUFFLEN - (size_t)(src - base) - n);
}

/*!
 * \brief insert a number of characters into a string buffer
 *
 * \param base base of the buffer (line or next)
 * \param src ptr to the position where to insert
 * \param n number of characters to insert
 */
static void str_insert(const char* base, char* src, size_t n)
{
	memmove(src + n, src, BUFFLEN - (size_t)(src - base) - n);
}

/*!
 * \brief convert one source file to doxygen style
 *
 * \param filename source filename (may include a path)
 * \return 0 on success, -1 on error
 *
 * The current function block comment format is sufficiently similar
 * enough for a number of automatic conversions to be done on the fly.
 *
 * The first occurence of a line starting with " *   " followed by
 * the basename of the filename (i.e. path stripped) makes this function
 * emit a doxygen style comment introducer "/ * !" followed by the original
 * line with the basename and "\\file" inserted before it.
 *
 * Following occurences of doxygen style comment introducers are used
 * to commence a simple state engine which detects:
 *   + "Input:" as the beginning of the function's parameters
 *   + "Return:" as the function's return value
 *   + "Note:" as a pre-formatted comment block
 *
 * The comments for each parameter are, currently, enclosed in parenthesis,
 * which this function removes.
 *
 * In some, rare cases also the parameters themselves are enclosed in
 * parenthesis, which contradicts most of the other comments. This is fixed
 * on the fly.
 *
 * Before each parameter (or list of parameters) with a comment, a string
 * "\param" is inserted. The parenthesis are used to detect mult-line
 * comments - doxygen joins them in the HTML output.
 *
 * The "Return:" is converted into doxygen "\return" and the parenthesis
 * around the comment, if any, are removed again.
 *
 * The "Note:" sections are prepended with a &lt;pre&gt; HTML tag line and
 * the entire remainder of the comment is closed with a &lt;/pre&gt;, just
 * before the end-of-comment sequence "* /"
 */
int scan_file(const char* filename)
{
	char backup[PATH_MAX];
	const char *basename;
	char *src;
	char *eol;
	struct stat st;
	FILE *fi, *fo;
	int state = 0;
	int pre = 0;
	int paren = 0;
	int fileblock = 0;

	basename = strrchr(filename, '/');
	if (basename)
		basename++;
	else
		basename = filename;

	snprintf(backup, sizeof(backup), "%s~", filename);
	if (stat(backup, &st) < 0) {
		char cmd[PATH_MAX];
		snprintf(cmd, sizeof(cmd), "cp %s %s", filename, backup);
		system(cmd);
	}
	fi = fopen(backup, "r");
	if (!fi) {
		fprintf(stderr, "Could not open '%s'\n", backup);
		return -1;
	}
	fo = fopen(filename, "w");
	if (!fo) {
		fprintf(stderr, "Could not create '%s'\n", filename);
		return -1;
	}
	if (NULL == fgets(next, sizeof(next), fi)) {
		fprintf(stderr, "Unexpected end of file in first line of '%s'\n", filename);
		return -1;
	}
	eol = strchr(next, '\n');
	if (eol)
		*eol = '\0';
	while (!feof(fi)) {
		memcpy(line, next, sizeof(line));
		if (NULL == fgets(next, sizeof(next), fi))
			break;
		eol = strchr(next, '\n');
		if (eol)
			*eol = '\0';
		if (!fileblock) {
			/* detect file block comment by " *    <filename.c>" formatted line */
			if (!strncmp(next, " *  ", 4) &&
			    (!strncmp(next + 4, basename, strlen(basename)) ||
			     !strncmp(next + 5, basename, strlen(basename)))) {
				/* insert "\file" before the filename in the next line */
				str_insert(next, next + 4, 5);
				memcpy(next + 3, "\\file ", 6);
				fileblock = 1;
			}
		}
		switch (state) {
		case 0:	/* normal source line */
			if (strncmp(line, "/*!", 3))
				break;
			state = 1;
			break;
		case 1: /* line after a doxygen comment start; insert "\brief" */
			src = line + 3;
			str_insert(line, src, 8);
			memcpy(src, "\\brief  ", 8);
			state = 2;
		case 2:	/* after start of doxygen comment search for "Input:" */
			if (strstr(line, "Input:") != line + 8)
				break;
			state = 3;
		case 3:	/* after "Input:" was detected, emit \param lines and search for "Return:" */
			if (strstr(line, "Notes:") == line + 4) {
				/* no "Return:", but "Notes:" following */
				fprintf(fo, " * <pre>\n");
				snprintf(line, sizeof(line), " * Notes:");
				pre = 1;
				state = 5;
				break;
			}
			if (strstr(line, "Return:") != line + 8) {
				memcpy(line + 7, "\\param ", 7);
				/* remove parenthesis around parameter names (format bug?) */
				src = line + 7 + 7;
				while (*src && isspace(*src))
					src++;
				if (*src == '(') {
					str_remove(line, src, 1);
					src = strchr(src, ')');
					if (src)
						str_remove(line, src, 1);
				}
				break;
			}
			memcpy(line + 7, "\\return ", 8);
			state = 4;
		case 4: // after "Return:" was detected, emit the "\return" line and look for "Notes:"
			if (strstr(line, "Notes:") != line + 4)
				break;
			fprintf(fo, " * <pre>\n");
			snprintf(line, sizeof(line), " * Notes:");
			pre = 1;
			state = 5;
		case 5: // after "Notes:" was detected look for end-of-comment
			if (!strstr(line, "*/"))
				break;
			if (pre) {
				fprintf(fo, " * </pre>\n");
				pre = 0;
			}
			state = 0;
		}

		/* handle (early) end-of-comment in states other than 5 */
		if (strstr(line, "*/")) {
			state = 0;
			pre = 0;
			paren = 0;
		}
		if (3 == state) {
			if (paren) {
				/* There was no closing parenthesis on the last line.
				 * This means the parameter comment extends to this line
				 * and there should be no "\param" in this line.
				 */
				memcpy(line + 7, "       ", 7);
			}
			/* convert any "<return optional>" into "<return> <optional>" */
			src = strstr(line, "<return optional>");
			if (src) {
				str_insert(line, src + 7, 2);
				memcpy(src + 7, "> <", 3);
			}
			/* convert any "<optional return>" into "<optional> <return>" */
			src = strstr(line, "<optional return>");
			if (src) {
				str_insert(line, src + 9, 2);
				memcpy(src + 9, "> <", 3);
			}
			/* convert any "<optional returns>" into "<optional> <return>" */
			src = strstr(line, "<optional returns>");
			if (src) {
				str_remove(line, src + 16, 1); /* change 'returns' into 'return' */
				str_insert(line, src + 9, 2);
				memcpy(src + 9, "> <", 3);
			}
			/* if line contains "<optional>", replace with "[optional]" */
			src = strstr(line, "<optional>");
			if (src) {
				src[0] = '[';
				src[9] = ']';
			}
			/* if line contains "<return>", replace "\param name" with "\param[out] name" */
			src = strstr(line, "<return>");
			if (src) {
				/* get rid of "<return> " */
				str_remove(line, src, 9);
				src = line + 7;	/* pos of \param in the line */
				src += 6;	/* length of "\param" */
				str_insert(line, src, 5);
				memcpy(src, "[out]", 5);
				src += 5;
				/* replace &param with pparam, the real parameter name */
				while (NULL != (src = strchr(src, '&')))
					*src = 'p';
			}
			/* if line contains "<inout>", replace "\param name" with "\param[in,out] name" */
			src = strstr(line, "<inout>");
			if (src) {
				/* get rid of "<inout> " */
				str_remove(line, src, 8);
				src = line + 7;	/* pos of \param in the line */
				src += 6;	/* length of "\param" */
				str_insert(line, src, 8);
				memcpy(src, "[in,out]", 8);
				src += 8;
				/* replace all &param with pparam, the real parameter name */
				while (NULL != (src = strchr(src, '&')))
					*src = 'p';
			}
			/* if line contains "\param ", replace with "\param[in]" */
			src = strstr(line, "\\param ");
			if (src) {
				src += 6;	/* length of "\param" */
				str_insert(line, src, 5);
				memcpy(src, "[in] ", 5);
				src += 5;
			}
		}
		if (3 == state || 4 == state) {
			if (!paren) {
				/* remove opening parenthesis of description, if any */
				src = strchr(line, '(');
				if (src) {
					str_remove(line, src, 1);
					paren = 1;
				}
			}
			if (paren) {
				/* remove closing parenthesis of description, if any */
				src = strrchr(line, ')');
				if (src) {
					str_remove(line, src, 1);
					paren = 0;
				}
			}
			/* remove leading spaces from "\param" and "\return" lines */
			src = line + 3;
			while (*src && isspace(*src))
				src++;
			if (src > line + 3 && *src == '\\') {
				memmove(line + 3, src, sizeof(line) - (size_t)(src - line));
			}
		}
		if (5 == state) {
			/* replace '<' with '\<' to avoid erroneous interpretation of HTML tags */
			src = line;
			while (NULL != (src = strchr(src, '<'))) {
				str_insert(line, src, 1);
				memcpy(src, "\\<", 2);
				src += 2;
			}
			/* replace '>' with '\>' to avoid erroneous interpretation of HTML tags */
			src = line;
			while (NULL != (src = strchr(src, '>'))) {
				str_insert(line, src, 1);
				memcpy(src, "\\>", 2);
				src += 2;
			}
			/* replace '&' with '\&' to avoid erroneous interpretation of HTML tags */
			src = line;
			while (NULL != (src = strchr(src, '&'))) {
				str_insert(line, src, 1);
				memcpy(src, "\\&", 2);
				src += 2;
			}
		}
		/* strip trailing spaces */
		src = line + strlen(line);
		while (src > line && isspace(src[-1]))
			*--src = '\0';
		if (1 == fileblock) {
			fprintf(fo, "/*!\n");
			fileblock = 2;
		} else if (2 == fileblock) {
			fprintf(fo, "%s\n", line);
			fprintf(fo, " * <pre>\n");
			pre = 1; /* insert " * </pre>" before end-of-comment */
			state = 5;
			fileblock = 3;
		} else {
			fprintf(fo, "%s\n", line);
		}
	}
	return 0;
}

/*!
 * \brief run conversion for all arguments given to the progam
 * \param argc argument count
 * \param argv array of argument strings (filenames)
 * \return 0 always
 */
int main(int argc, char** argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (scan_file(argv[i]) < 0)
			break;
	}
	return 0;
}
