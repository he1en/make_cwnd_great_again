#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

static const double ALPHA = 0.8;

/* Default constructor */
Controller::Controller( const bool debug, const unsigned int window_size )
  : debug_( debug ),
    window_size_((float) 14 + window_size * 0),
    last_acked_num_(0),
    rtt_estimate_(20)
{
    // Nada
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << window_size_ << endl;
  }

  return (int) window_size_;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                    /* in milliseconds */
                    bool from_timeout )
{
  

  if ( from_timeout ) {
      /* add timeout bool arg here and to DatagrumpSEnder::send_datagram
         and half the cwnd if datagramis being resent because of a timeout */
    window_size_ /= 2;
    if ( window_size_ < 1) {
      window_size_ = 1;
    }
  }

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number
     << " because of a timeout " << (from_timeout ? "yes" : "no") << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
    uint64_t observed_rtt = timestamp_ack_received - send_timestamp_acked;
    cout << "Observed rtt: " << observed_rtt << endl;
    rtt_estimate_ = ALPHA * rtt_estimate_ + (1 - ALPHA) * observed_rtt;
    cout << "New rtt estimate: " << rtt_estimate_ << endl;

    if ( sequence_number_acked > last_acked_num_) {
        // additive increase
        last_acked_num_ = sequence_number_acked;
        
        float delta = window_size_ <= 1 ? 1 : 1 / window_size_;
        window_size_ += delta;
    }
  
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 200; // (int) rtt_estimate_;
}
