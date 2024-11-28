#include "array_rank.h"

namespace spiritsaway::system::rank
{
	array_rank::array_rank(const std::string& name, std::uint32_t rank_sz, std::uint32_t pool_sz, double min_value, double max_value)
		: rank_interface(name, rank_sz, pool_sz, min_value, max_value)
		, m_rank_info_pool(pool_sz)
		, m_pool_unused_indexes(pool_sz)
	{
		for (std::uint32_t i = 0; i < pool_sz; i++)
		{
			m_pool_unused_indexes[i] = i;
		}
		std::reverse(m_pool_unused_indexes.begin(), m_pool_unused_indexes.end());
		m_player_to_pool_idx.reserve(2 * m_pool_sz);
		m_sorted_rank_ptrs.reserve(m_pool_sz);
	}
	bool array_rank::remove(const std::string &player_id)
	{
		auto temp_iter = m_player_to_pool_idx.find(player_id);
		if (temp_iter == m_player_to_pool_idx.end())
		{
			return false;
		}
		const auto &one_player = m_rank_info_pool[temp_iter->second];

		auto vec_iter = std::lower_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), rank_info_ptr_wrapper{&one_player});
		m_sorted_rank_ptrs.erase(vec_iter);
		m_rank_info_pool[temp_iter->second].clear();
		m_pool_unused_indexes.push_back(temp_iter->second);
		m_player_to_pool_idx.erase(temp_iter);
		return true;
	}
	std::pair<const rank_info*, std::uint32_t> array_rank::get_rank(const std::string& player_id) const 
	{
		auto temp_iter = m_player_to_pool_idx.find(player_id);
		if (temp_iter == m_player_to_pool_idx.end())
		{
			return std::make_pair(nullptr, 0);
		}
		else
		{
			const auto& one_player = m_rank_info_pool[temp_iter->second];

			auto vec_iter = std::lower_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), rank_info_ptr_wrapper{ &one_player });
			auto cur_rank = std::distance(m_sorted_rank_ptrs.begin(), vec_iter);
			if (cur_rank >= m_rank_sz)
			{
				return std::make_pair(&m_rank_info_pool[temp_iter->second], 0);
			}
			else
			{
				return std::make_pair(&m_rank_info_pool[temp_iter->second], cur_rank + 1);
			}
		}
	}
	std::uint32_t array_rank::get_rank(double rank_value) const
	{
		rank_info temp_rank_info;
		temp_rank_info.rank_value = rank_value;
		temp_rank_info.update_ts = std::numeric_limits<std::uint64_t>::max();
		auto vec_iter = std::upper_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), rank_info_ptr_wrapper{&temp_rank_info});
		auto cur_rank = std::distance(m_sorted_rank_ptrs.begin(), vec_iter);
		if (cur_rank >= m_rank_sz)
		{
			return 0;
		}
		else
		{
			return cur_rank + 1;
		}
	}
	void array_rank::reset(const std::vector<rank_info> &player_ranks)
	{
		m_player_to_pool_idx.clear();
		m_sorted_rank_ptrs.clear();
		for (auto &one_rank : m_rank_info_pool)
		{
			one_rank.clear();
		}
		m_pool_unused_indexes.clear();
		for (std::uint32_t i = 0; i < m_pool_sz; i++)
		{
			m_pool_unused_indexes.push_back(i);
		}
		std::reverse(m_pool_unused_indexes.begin(), m_pool_unused_indexes.end());
		for (const auto &one_player : player_ranks)
		{
			if (m_pool_unused_indexes.empty())
			{
				return;
			}
			auto cur_idx = m_pool_unused_indexes.back();
			m_pool_unused_indexes.pop_back();
			m_rank_info_pool[cur_idx] = one_player;
			m_sorted_rank_ptrs.push_back(rank_info_ptr_wrapper{&m_rank_info_pool[cur_idx]});
			m_player_to_pool_idx[one_player.player_id] = cur_idx;
		}
	}

	std::uint32_t array_rank::update(const rank_info &one_player)
	{
		
		auto temp_iter = m_player_to_pool_idx.find(one_player.player_id);

		if (temp_iter == m_player_to_pool_idx.end())
		{
			if (one_player.rank_value <= m_min_value || one_player.rank_value >= m_max_value)
			{
				return 0;
			}
			if (m_sorted_rank_ptrs.size() == m_pool_sz)
			{
				if (*m_sorted_rank_ptrs.back().ptr < one_player)
				{
					return 0;
				}
				else
				{
					auto pre_idx_iter = m_player_to_pool_idx.find(m_sorted_rank_ptrs.back().ptr->player_id);
					m_pool_unused_indexes.push_back(pre_idx_iter->second);
					m_rank_info_pool[pre_idx_iter->second].clear();
					m_player_to_pool_idx.erase(pre_idx_iter);
					m_sorted_rank_ptrs.pop_back();
				}
			}
			auto cur_player_idx = m_pool_unused_indexes.back();
			m_pool_unused_indexes.pop_back();
			m_player_to_pool_idx[one_player.player_id] = cur_player_idx;

			m_rank_info_pool[cur_player_idx] = one_player;
			m_rank_info_pool[cur_player_idx].update_ts = gen_next_update_ts();
			auto cur_key = rank_info_ptr_wrapper{&m_rank_info_pool[cur_player_idx]};

			auto cur_insert_iter = std::lower_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), cur_key);
			auto result_iter = m_sorted_rank_ptrs.insert(cur_insert_iter, cur_key);
			return std::distance(m_sorted_rank_ptrs.begin(), result_iter) + 1;
		}
		if (one_player.rank_value <= m_min_value || one_player.rank_value >= m_max_value)
		{
			remove(one_player.player_id);
			return 0;
		}
		
		auto cur_key = rank_info_ptr_wrapper{&one_player};
		auto pre_key = rank_info_ptr_wrapper{&m_rank_info_pool[temp_iter->second]};
		bool is_key_increase = pre_key < cur_key;
		auto self_iter = std::lower_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), pre_key);
		if (m_rank_info_pool[temp_iter->second].rank_value == one_player.rank_value)
		{
			return std::distance(m_sorted_rank_ptrs.begin(), self_iter) + 1;
		}
		m_rank_info_pool[temp_iter->second] = one_player;
		m_rank_info_pool[temp_iter->second].update_ts = gen_next_update_ts();
		if (is_key_increase)
		{
			auto next_iter = self_iter + 1;
			while (next_iter != m_sorted_rank_ptrs.end() && (*next_iter < pre_key))
			{
				std::swap(*next_iter, *self_iter);
				next_iter = next_iter + 1;
				self_iter++;
			}
		}
		else
		{
			while (self_iter != m_sorted_rank_ptrs.begin())
			{
				auto pre_iter = self_iter - 1;
				if (*pre_iter < pre_key)
				{
					break;
				}
				std::swap(*pre_iter, *self_iter);
				self_iter = pre_iter;
			}
		}
		return std::distance(m_sorted_rank_ptrs.begin(), self_iter) + 1;
	}

	json array_rank::encode() const
	{
		std::vector<rank_info> temp_sorted_rank_info;
		temp_sorted_rank_info.reserve(m_sorted_rank_ptrs.size());
		for (const auto &one_key : m_sorted_rank_ptrs)
		{
			temp_sorted_rank_info.push_back(*one_key.ptr);
		}
		json result = rank_interface::encode();
		result["sorted_ranks"] = temp_sorted_rank_info;
		return result;
	}

	bool array_rank::decode(const json &data)
	{
		std::uint32_t rank_sz;
		std::uint32_t pool_sz;
		std::string name;
		std::vector<rank_info> temp_sorted_rank_info;
		try
		{
			data.at("name").get_to(name);
			data.at("pool_sz").get_to(pool_sz);
			data.at("rank_sz").get_to(rank_sz);
			data.at("sorted_ranks").get_to(temp_sorted_rank_info);
		}
		catch (std::exception &e)
		{
			assert(false);
			return false;
		}

		reset(temp_sorted_rank_info);
		return true;
	}
	bool array_rank::update_player_info(const std::string &player_id, const json::object_t &player_info)
	{
		auto temp_iter = m_player_to_pool_idx.find(player_id);
		if (temp_iter == m_player_to_pool_idx.end())
		{
			return false;
		}
		m_rank_info_pool[temp_iter->second].player_info = player_info;
		return true;
	}

	const rank_info* array_rank::get_player(std::uint32_t in_rank) const
	{
		if (in_rank == 0 || in_rank > m_rank_sz || in_rank > m_sorted_rank_ptrs.size())
		{
			return nullptr;
		}
		return m_sorted_rank_ptrs[in_rank - 1].ptr;
	}

	std::vector<const rank_info*> array_rank::get_players(std::uint32_t begin_rank, std::uint32_t end_rank) const
	{
		if (begin_rank > end_rank)
		{
			return {};
		}
		if (begin_rank == 0 || begin_rank > m_rank_sz || begin_rank > m_sorted_rank_ptrs.size())
		{
			return {};
		}

		end_rank = std::min(std::min(end_rank, m_rank_sz), std::uint32_t(m_sorted_rank_ptrs.size()));
		std::vector<const rank_info*> result(end_rank - begin_rank + 1);
		for (std::uint32_t i = begin_rank; i <= end_rank; i++)
		{
			result[i] = m_sorted_rank_ptrs[i - 1].ptr;
		}
		return result;
	}
}