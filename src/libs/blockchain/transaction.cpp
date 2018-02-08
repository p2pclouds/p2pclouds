#include "transaction.h"

namespace P2pClouds {

	Transaction::Transaction()
		: sender_()
		, recipient_()
		, amount_(0)
	{
	}

	Transaction::~Transaction()
	{
	}

}
