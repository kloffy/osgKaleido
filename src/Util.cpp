#include <osgKaleido/Util>

#include <cstdio>
#include <cstdarg>

#define OSGKALEIDO_FORMAT_BUFFER_SIZE 1024

namespace osgKaleido {

std::string format(const char *fmt, ...)
{
	char buffer[OSGKALEIDO_FORMAT_BUFFER_SIZE];
	va_list arg;
	va_start(arg, fmt);
	vsprintf(buffer, fmt, arg);
	va_end(arg);
	return std::string(buffer);
}

#ifdef OSGKALEIDO_EXCEPTIONS_ENABLED
void throw_exception()
{
	throw std::runtime_error(format("Error generating polyhedron data: \"%s\".", error.message));
}
#else
void throw_exception()
{
	//Do nothing. The application should call getStatus() instead.
}
#endif // OSGKALEIDO_EXCEPTIONS_ENABLED

} // osgKaleido