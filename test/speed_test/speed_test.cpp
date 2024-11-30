#include "array_rank.h"

#include "skiplist_rank.h"

#include <random>
#include <chrono>

using namespace spiritsaway::system::rank;

struct cost_reporter
{
	static std::unordered_map<std::string, double> stats;
	decltype(std::chrono::high_resolution_clock::now()) begin_ts;
	const std::string name;
	cost_reporter(std::string in_name)
		: name(in_name)
	{
		begin_ts = std::chrono::high_resolution_clock::now();
	}

	~cost_reporter()
	{
		auto end_ts = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds{ end_ts - begin_ts };
		std::cout << elapsed_seconds.count() * 1000 << ",";
		stats[name] += elapsed_seconds.count() * 1000;
	}

};
std::unordered_map<std::string, double> cost_reporter::stats;
void test_2(std::vector<std::shared_ptr<rank_interface>> rank_impls, const std::vector<rank_info>& temp_ranks, const std::string& name)
{
	for (int i = 0; i < rank_impls.size(); i++)
	{
		{
			auto reporter = cost_reporter(rank_impls[i]->rank_impl_name() + "_" + name + "_update");
			for (const auto& one_info : temp_ranks)
			{
				rank_impls[i]->update(one_info);
			}
		}
		{
			auto reporter = cost_reporter(rank_impls[i]->rank_impl_name() + "_" + name + "_query");
			for (const auto& one_info : temp_ranks)
			{
				rank_impls[i]->get_rank(one_info.player_id);
			}
		}
		{
			auto reporter = cost_reporter(rank_impls[i]->rank_impl_name() + "_" + name + "_remove");
			for (const auto& one_info : temp_ranks)
			{
				rank_impls[i]->remove(one_info.player_id);
			}
		}
	}
}
void test_1(std::uint32_t num)
{
	std::uint32_t rank_num = num;
	std::uint32_t pool_num = num * 1.5;;
	std::vector<rank_info> temp_ranks;
	for (int i = 0; i <  4 * pool_num; i++)
	{
		rank_info cur_rank_info;
		cur_rank_info.player_id = "player_" + std::to_string(i);
		cur_rank_info.rank_value = -1.0 * i;
		temp_ranks.push_back(cur_rank_info);
	}
	std::cout << num << ",";
	std::random_device r;
	std::default_random_engine e1(r());
	std::vector< std::shared_ptr<rank_interface>> rank_impls;
	{
		auto reporter = cost_reporter("array_create" + std::to_string(num));
		rank_impls.push_back(std::make_shared<array_rank>("temp", rank_num, pool_num, -10 * pool_num, 10 * pool_num));

	}
	{
		auto reporter = cost_reporter("list_create" +std::to_string(num));
		rank_impls.push_back(std::make_shared<skiplist_rank>("temp", rank_num, pool_num, -10 * pool_num, 10 * pool_num));

	}
	std::sort(temp_ranks.begin(), temp_ranks.end());

	test_2(rank_impls, temp_ranks, "sorted" + std::to_string(num));

	std::reverse(temp_ranks.begin(), temp_ranks.end());

	test_2(rank_impls, temp_ranks, "reverse" + std::to_string(num));

	for (int i = 0; i < 1; i++)
	{
		std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
		
		test_2(rank_impls, temp_ranks, "random"+std::to_string(num));

		

	}
	std::cout << "\n";
}

int main()
{
	std::cout << "count,array_create,list_create,array_update_sorted,list_update_sorted,array_query_sorted,list_query_sorted,array_remove_sorted,list_remove_sorted,array_update_reverse,list_update_reverse,array_query_reverse,list_query_reverse,array_remove_reverse,list_remove_reverse,array_update_random,list_update_random,array_query_random,list_query_random,array_remove_random,list_remove_random" << std::endl;
	for (int i = 1; i < 10; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			test_1(i * 100000);
		}
		

		//for (const auto& one_stat : cost_reporter::stats)
		//{
		//	std::cout << one_stat.first << "," << one_stat.second << std::endl;
		//}
		//cost_reporter::stats.clear();
	}
	return 1;
}