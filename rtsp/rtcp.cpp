
#include "stdafx.h"
#include "rtp.h"
#include "rtcp.h"


void init_seq(source *s, uint16_t seq)
{
	s->base_seq = seq;
	s->max_seq = seq;
	s->bad_seq = RTP_SEQ_MOD + 1;   /* so seq == bad_seq is false */
	s->cycles = 0;
	s->received = 0;
	s->received_prior = 0;
	s->expected_prior = 0;
	/* other initialization */
}

int update_seq(source *s, uint16_t seq)
{
	uint16_t udelta = seq - s->max_seq;
	const int32_t MAX_DROPOUT = 3000;
	const int32_t MAX_MISORDER = 100;
	const int32_t MIN_SEQUENTIAL = 2;
	/*
	* Source is not valid until MIN_SEQUENTIAL packets with
	* sequential sequence numbers have been received.
	*/
	if (s->probation) {
		/* packet is in sequence */
		if (seq == s->max_seq + 1) {
			s->probation--;
			s->max_seq = seq;
			if (s->probation == 0) {
				init_seq(s, seq);
				s->received++;
				return 1;
			}
		}
		else {
			s->probation = MIN_SEQUENTIAL - 1;
			s->max_seq = seq;
		}
		return 0;
	}
	else if (udelta < MAX_DROPOUT) {
		/* in order, with permissible gap */
		if (seq < s->max_seq) {
			/*
			* Sequence number wrapped - count another 64K cycle.
			*/
			s->cycles += RTP_SEQ_MOD;
		}
		s->max_seq = seq;
	}
	else if (udelta <= RTP_SEQ_MOD - MAX_MISORDER) {
		/* the sequence number made a very large jump */
		if (seq == s->bad_seq) {
			/*
			* Two sequential packets -- assume that the other side
			* restarted without telling us so just re-sync
			* (i.e., pretend this was the first packet).
			*/
			init_seq(s, seq);
		}
		else {
			s->bad_seq = (seq + 1) & (RTP_SEQ_MOD - 1);
			return 0;
		}
	}
	else {
		/* duplicate or reordered packet */
	}
	s->received++;
	return 1;
}


