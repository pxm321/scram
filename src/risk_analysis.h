// Classes for various risk analysis.

#ifndef SCRAM_RISK_ANALYISIS_H_
#define SCRAM_RISK_ANALYISIS_H_

#include <map>
#include <set>
#include <string>
#include <queue>

#include <boost/serialization/map.hpp>
#include <boost/unordered_map.hpp>

#include "event.h"

namespace scram {

class Superset;

// Interface for various risk analysis methods.
class RiskAnalysis {
 public:
  virtual void ProcessInput(std::string input_file) = 0;
  virtual void PopulateProbabilities(std::string prob_file) = 0;
  virtual void GraphingInstructions() = 0;
  virtual void Analyze() = 0;
  virtual void Report(std::string output) = 0;

  virtual ~RiskAnalysis() {}
};


// Fault tree analysis.
class FaultTree : public RiskAnalysis {
  // This is a set class with various options which needs access to the private.
  friend class Superset;

 public:
  FaultTree(std::string analysis, bool graph_only,  bool rare_event = false,
            int limit_order = 20, int nsums = 1000000);

  // Reads input file with the structure of the Fault tree.
  // Puts all events into their appropriate containers.
  void ProcessInput(std::string input_file);

  // Reads probabiliteis for primary events from formatted input file with values.
  // Attaches probabilities to primary events.
  void PopulateProbabilities(std::string prob_file);

  // Outputs a file with instructions for graphviz dot to create a fault tree.
  void GraphingInstructions();

  // Analyzes the fault tree and performs computations.
  void Analyze();

  // Reports the results of analysis to a specified output destination.
  void Report(std::string output);

  virtual ~FaultTree() {}

 private:
  // Gets arguments from a line in an input file formatted accordingly.
  bool GetArgs_(std::vector<std::string>& args, std::string& line,
                std::string& orig_line);

  // Interpret arguments and perform specific actions.
  void InterpretArgs_(int nline, std::stringstream& msg,
                      std::vector<std::string>& args,
                      std::string& orig_line,
                      std::string tr_parent = "",
                      std::string tr_id = "",
                      std::string suffix = "");

  // Adds node and updates databases.
  void AddNode_(std::string parent, std::string id, std::string type);

  // Adds probability to a primary event.
  void AddProb_(std::string id, double p);

  // Includes external transfer in subtrees to this current main tree.
  void IncludeTransfers_();

  // Adds children of top or intermediate event into a specified vector of sets.
  void ExpandSets_(scram::TopEvent* t,
                   std::vector< Superset* >& sets);

  // Verifies if gates are initialized correctly with right number of children.
  // Returns a warning message string with the list of bad gates and their
  // problems.
  // Returns an empty string if no problems are detected.
  std::string CheckGates_();

  // Returns primary events that do not have probabilities assigned.
  std::string PrimariesNoProb_();

  // Calculates a probability of a minimal cut set, which members are in AND
  // relationship with each other. This function assumes independence of each
  // member.
  double ProbAnd_(const std::set<std::string>& min_cut_set);

  // -------------------- Algorithm Improvement Trial:Integers ---------------
  // Calculates a probability of a set of minimal cut sets, which are in OR
  // relationship with each other. This function is a brute force probability
  // calculation without rare event approximations.
  // nsums parameter specifies number of sums in the series.
  double ProbOr_(std::set< std::set<int> >& min_cut_sets,
                 int nsums = 1000000);

  // Calculates a probability of a minimal cut set, which members are in AND
  // relationship with each other. This function assumes independence of each
  // member.
  double ProbAnd_(const std::set<int>& min_cut_set);

  // Calculates A(and)( B(or)C ) relationship for sets using set algebra.
  // Returns non-const reference because only intended to be used for
  // brute force probability calculations.
  void CombineElAndSet_(const std::set<int>& el,
                        const std::set< std::set<int> >& set,
                        std::set< std::set<int> >& combo_set);

  std::set< std::set<int> > imcs_;
  boost::unordered_map<int, scram::PrimaryEvent*> int_to_prime_;
  boost::unordered_map<std::string, int> prime_to_int_;
  std::vector<double> iprobs_;  // holds probabilities of basic events.
  // -----------------------------------------------------------------
  // ---- Algorithm for Equation Construction for Monte Carlo Sim -------
  void MProbOr_(std::set< std::set<int> >& min_cut_sets, int sign,
                 int nsums = 1000000);

  void MCombineElAndSet_(const std::set<int>& el,
                        const std::set< std::set<int> >& set,
                        std::set< std::set<int> >& combo_set);

  void MSample();  // perform simulation.
  std::vector< std::set<int> > pos_terms_;  // plus terms of the equation.
  std::vector< std::set<int> > neg_terms_;  // minus terms of the equation.
  std::vector<double> sampled_results_;  // storage for sampled values.
  int nsimulations_;  // number of simulations to run.
  // -----------------------------------------------------------------
  // This member is used to provide any warnings about assumptions,
  // calculations, and settings. These warnings must be written into output
  // file.
  std::string warnings_;

  // Type of analysis to be performed.
  std::string analysis_;

  // Request for graphing instructions only.
  bool graph_only_;

  // Rare event approximation.
  bool rare_event_;

  // Input file path..
  std::string input_file_;

  // Keep track of currently opened file with sub-trees.
  std::string current_file_;

  // Indicator if probability calculations are requested.
  bool prob_requested_;

  // Number of sums in series expansion for probability calculations.
  int nsums_;

  // Container of original names of events with capitalizations.
  std::map<std::string, std::string> orig_ids_;

  // List of all valid gates.
  std::set<std::string> gates_;

  // List of all valid types of primary events.
  std::set<std::string> types_;

  // Id of a top event.
  std::string top_event_id_;

  // Top event.
  scram::TopEvent* top_event_;

  // Indicator of detection of a top event described by a transfer sub-tree.
  bool top_detected_;

  // Indicates that reading the main tree file as opposed to a transfer tree.
  bool is_main_;

  // Holder for intermediate events.
  boost::unordered_map<std::string, scram::InterEvent*> inter_events_;

  // Container for primary events.
  boost::unordered_map<std::string, scram::PrimaryEvent*> primary_events_;

  // Container for transfer symbols as requested in tree initialization.
  // A queue contains a tuple of the parent and id of transferIn.
  std::queue< std::pair<std::string, std::string> > transfers_;

  // For graphing purposes the same transferIn.
  std::multimap<std::string, std::string> transfer_map_;

  // Container for storing all transfer sub-trees' names and number of calls.
  std::map<std::string, int> trans_calls_;

  // Container to track transfer calls to prevent cyclic calls/inclusions.
  std::map< std::string, std::vector<std::string> > trans_tree_;

  // Container for minimal cut sets.
  std::set< std::set<std::string> > min_cut_sets_;

  // Container for minimal cut sets and their respective probabilities.
  std::map< std::set<std::string>, double > prob_of_min_sets_;

  // Container for minimal cut sets ordered by their probabilities.
  std::multimap < double, std::set<std::string> > ordered_min_sets_;

  // Container for primary events and their contribution.
  std::map< std::string, double > imp_of_primaries_;

  // Container for primary events ordered by their contribution.
  std::multimap < double, std::string > ordered_primaries_;

  // Maximum order of the minimal cut sets.
  int max_order_;

  // Limit on the size of the minimal cut sets for performance reasons.
  int limit_order_;

  // Total probability of the top event.
  double p_total_;

  // Specific variables that are shared for initialization of tree nodes.
  std::string parent_;
  std::string id_;
  std::string type_;
  bool block_started_;
  // Indicate if TransferOut is initiated correctly.
  bool transfer_correct_;

  // Indication of the first intermediate event of the transfer.
  bool transfer_first_inter_;
};

}  // namespace scram

#endif  // SCRAM_RISK_ANALYSIS_H_
