#include "pch.h"
#include "RecvBuffer.h"


RecvBuffer::RecvBuffer(int32 bufferSize) : _bufferSize(bufferSize)
{
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clean()
{
	int32 dataSize = DataSize();
	if (dataSize == 0)
	{
		// read, write 커서가 같은 위치라면
		_readPos = _writePos = 0;
		return;
	}

	if (FreeSize() < _bufferSize)
	{
		// 여유 공간이 버퍼 1개 크기 미만이라면
		::memcpy(&_buffer[0], &_buffer[_readPos], dataSize);

		_readPos = 0;
		_writePos = dataSize;
	}
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize())
	{
		return false;
	}

	_readPos += numOfBytes;
	return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize())
	{
		return false;
	}

	_writePos += numOfBytes;
	return true;
}
