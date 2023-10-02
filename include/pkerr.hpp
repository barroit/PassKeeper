#pragma once

class ValidateException : public std::runtime_error
{
public:
	explicit ValidateException(const std::string &message)
			: std::runtime_error{ message }
	{}
};

class ORMException : public std::runtime_error
{
public:
	explicit ORMException(const std::string &message)
			: std::runtime_error{ message }
	{}
};
