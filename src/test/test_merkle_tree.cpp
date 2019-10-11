 #include "stubs.hpp"
#include "gadgets/merkle_tree.hpp"
#include "gadgets/mimc.hpp"

namespace ethsnarks {


bool test_merkle_path_selector(int is_right)
{
	ProtoboardT pb;

	const auto value_A = FieldT("149674538925118052205057075966660054952481571156186698930522557832224430770");
	const auto value_B = FieldT("9670701465464311903249220692483401938888498641874948577387207195814981706974");

	is_right = is_right ? 1 : 0;

	VariableT var_A = make_variable(pb, "var_A");
	pb.val(var_A) = value_A;

	VariableT var_B = make_variable(pb, "var_B");
	pb.val(var_B) = value_B;

	VariableT var_is_right = make_variable(pb, "var_is_right");
	pb.val(var_is_right) = is_right;

	merkle_path_selector selector(pb, var_A, var_B, var_is_right, "test_merkle_path_selector");

	selector.generate_r1cs_witness();
	selector.generate_r1cs_constraints();

	if( is_right ) {
		if( pb.val(selector.left()) != value_B ) {
			return false;
		}
		if( pb.val(selector.right()) != value_A ) {
			return false;
		}
	}
	else {
		if( pb.val(selector.left()) != value_A ) {
			return false;
		}
		if( pb.val(selector.right()) != value_B ) {
			return false;
		}
	}

	if( ! pb.is_satisfied() ) {
		std::cerr << "FAIL merkle_path_authenticator is_satisfied\n";
		return false;
	}

	return stub_test_proof_verify(pb);
}


bool test_merkle_path_authenticator() {
    // Tree depth is 1, left leaf is 3703141493535563179657531719960160174296085208671919316200479060314459804651,
    // right leaf is 134551314051432487569247388144051420116740427803855572138106146683954151557,
    // root is 3075442268020138823380831368198734873612490112867968717790651410945045657947. Authenticator for right leaf (`is_right` = 1)

    auto left = FieldT("3703141493535563179657531719960160174296085208671919316200479060314459804651");
    auto right = FieldT("134551314051432487569247388144051420116740427803855572138106146683954151557");
    auto root = FieldT("3075442268020138823380831368198734873612490112867968717790651410945045657947");
    auto is_right = 1;

	ProtoboardT pb;

	VariableArrayT address_bits;
	address_bits.allocate(pb, 1, "address_bits");
	pb.val(address_bits[0]) = is_right;

	VariableArrayT path;
	path.allocate(pb, 1, "path");
	pb.val(path[0]) = left;

	VariableT leaf;
	leaf.allocate(pb, "leaf");
	pb.val(leaf) = right;

	VariableT expected_root;
	expected_root.allocate(pb, "expected_root");
	pb.val(expected_root) = root;

	size_t tree_depth = 1;
	merkle_path_authenticator<MiMC_e7_hash_gadget> auth(
		pb, tree_depth, address_bits,
		merkle_tree_IVs(pb),
		leaf, expected_root, path,
		"authenticator");

	auth.generate_r1cs_witness();
	auth.generate_r1cs_constraints();

	if( ! auth.is_valid() ) {
		std::cerr << "Not valid!" << std::endl;
		std::cerr << "Expected "; pb.val(expected_root).print();
		std::cerr << "Actual "; pb.val(auth.result()).print();
		return false;
	}

	if( ! pb.is_satisfied() ) {
		std::cerr << "Not satisfied!" << std::endl;
		return false;
	}

	return true;
}

// namespace ethsnarks
}


int main( int argc, char **argv )
{
    ethsnarks::ppT::init_public_params();

    if( ! ethsnarks::test_merkle_path_authenticator() )
    {
        std::cerr << "FAIL merkle_path_authenticator\n";
        return 1;
    }

    if( ! ethsnarks::test_merkle_path_selector(0) )
    {
        std::cerr << "FAIL merkle_path_selector 0\n";
        return 2;
    }

    if( ! ethsnarks::test_merkle_path_selector(1) )
    {
        std::cerr << "FAIL merkle_path_selector 1\n";
        return 2;
    }

    std::cout << "OK\n";
    return 0;
}

