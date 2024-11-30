#include "skiplist_rank.h"

namespace spiritsaway::system::rank
{
	skiplist_rank::skiplist_rank(const std::string& name, std::uint32_t rank_sz, std::uint32_t pool_sz, double min_value, double max_value)
		: rank_interface(name, rank_sz, pool_sz, min_value, max_value)
		, m_max_node(&m_max_rank_info, MAX_LEVEL, false)
		, m_min_node(&m_min_rank_info, MAX_LEVEL, false)
		, m_level(0)
		, m_random_engine(std::random_device{}()), m_random_dis(0, (1ull << (2 * MAX_LEVEL)) - 1)
	{
		m_max_rank_info.rank_value = max_value;
		m_max_rank_info.update_ts = 0;
		m_min_rank_info.rank_value = min_value;
		m_min_rank_info.update_ts = std::numeric_limits<std::uint64_t>::max();

		for (int i = 0; i < MAX_LEVEL; i++)
		{
			m_max_node.nexts[i] = &m_min_node;
			m_max_node.spans[i] = 0;
		}
		m_key_to_node.reserve(2 * pool_sz);
		m_prev_node_for_min = &m_max_node;
	}

	skiplist_rank::~skiplist_rank()
	{
		m_max_node.clear();
		m_min_node.clear();
		m_key_to_node.clear();
	}

	int skiplist_rank::random_level()
	{
		std::uint64_t temp_random_value = m_random_dis(m_random_engine);
		int result_level = 0;
		while (temp_random_value)
		{
			// 每上升一层的概率为0.25 这样可以使得每一层的节点数量是下一层的一半
			if (temp_random_value % 4 == 3)
			{
				result_level++;
				temp_random_value /= 4;
			}
			else
			{
				break;
			}
		}
		if (result_level > MAX_LEVEL - 1)
		{
			result_level = MAX_LEVEL - 1;
		}
		if (result_level > m_level)
		{
			// 超过当前最大层数的时候直接对当前层数加1
			result_level = ++m_level;
			m_max_node.spans[result_level] = size();
		}
		return result_level;
	}
	skiplist_rank::node *skiplist_rank::create_node(const rank_info &one_player)
	{
		auto temp_node = std::make_unique<node>(new rank_info(one_player), random_level());
		auto result = temp_node.get();
		result->rank_info_ptr->update_ts = gen_next_update_ts();
		m_key_to_node[one_player.player_id] = std::move(temp_node);
		return result;
	}
	skiplist_rank::node *skiplist_rank::find_node(const std::string &key) const
	{
		auto temp_iter = m_key_to_node.find(key);
		if (temp_iter == m_key_to_node.end())
		{
			return nullptr;
		}
		else
		{
			return temp_iter->second.get();
		}
	}

	// 记录每一层中当前node 的prev节点
	void skiplist_rank::get_prev_nodes(const node &in_node, std::array<node *, MAX_LEVEL> &prev_nodes, std::array<std::uint32_t, MAX_LEVEL> &prev_ranks)
	{
		//std::fill(prev_nodes.begin(), prev_nodes.begin() + m_level + 1, nullptr);
		//std::fill(prev_ranks.begin(), prev_ranks.begin() + m_level + 1, 0);
		node *search_node = &m_max_node;
		std::uint32_t last_level_prev_rank = 0;
		for (int i = m_level; i >= 0; --i)
		{

			// 由于我们保证了最末尾有一个m_min_node
			// 所以search_node->next[i]永远存在
			while (*search_node->nexts[i] < in_node)
			{
				last_level_prev_rank += search_node->spans[i] + 1;
				search_node = search_node->nexts[i];
			}
			prev_ranks[i] = last_level_prev_rank;
			prev_nodes[i] = search_node;
		}
	}
	// 记录每一层中当前node 的prev节点
	void skiplist_rank::get_prev_nodes_const(const node &in_node, std::array<const node *, MAX_LEVEL> &prev_nodes, std::array<std::uint32_t, MAX_LEVEL> &prev_ranks) const
	{
		//std::fill(prev_nodes.begin(), prev_nodes.begin() + m_level + 1, nullptr);
		//std::fill(prev_ranks.begin(), prev_ranks.begin() + m_level + 1, 0);
		const node *search_node = &m_max_node;
		std::uint32_t last_level_prev_rank = 0;
		for (int i = m_level; i >= 0; --i)
		{

			// 由于我们保证了最末尾有一个m_min_node
			// 所以search_node->next[i]永远存在
			while (*search_node->nexts[i] < in_node)
			{
				last_level_prev_rank += search_node->spans[i] + 1;
				search_node = search_node->nexts[i];
			}
			prev_ranks[i] = last_level_prev_rank;
			prev_nodes[i] = search_node;
		}
	}
	void skiplist_rank::remove_from_list(node *cur_node)
	{

		std::array<node *, MAX_LEVEL> prev_nodes;
		std::array<std::uint32_t, MAX_LEVEL> prev_ranks;
		get_prev_nodes(*cur_node, prev_nodes, prev_ranks);
		for (int i = 0; i <= m_level; i++)
		{
			if (prev_nodes[i]->nexts[i] == cur_node)
			{
				prev_nodes[i]->spans[i] += cur_node->spans[i];
				prev_nodes[i]->nexts[i] = cur_node->nexts[i];
			}
			else
			{
				prev_nodes[i]->spans[i] -= 1;
			}
		}
		if (cur_node == m_prev_node_for_min)
		{
			m_prev_node_for_min = prev_nodes[0];
		}
		cur_node->clear();
	}

	// 返回插入之后的排名
	std::uint32_t skiplist_rank::update(const rank_info &one_player)
	{
		// debug_print();
		auto pre_size = size();
		auto temp_node = find_node(one_player.player_id);
		if (temp_node == nullptr)
		{
			if (one_player.rank_value <= m_min_rank_info.rank_value || one_player.rank_value >= m_max_rank_info.rank_value)
			{
				// 不在范围之内的则不参与排名
				return 0;
			}
			if (pre_size == m_pool_sz)
			{
				// 如果满了 且当前值大于最末尾的有效值 则不参与排名
				if (*m_prev_node_for_min < one_player.rank_value)
				{
					return m_pool_sz + 1;
				}
			}
			temp_node = create_node(one_player);
		}
		else
		{
			if (temp_node->rank_info_ptr->rank_value == one_player.rank_value)
			{
				// 相同分数 直接返回排名
				return get_rank(one_player.player_id).second;
			}
			else
			{
				remove_from_list(temp_node);
				pre_size -= 1;
				if (one_player.rank_value <= m_min_rank_info.rank_value || one_player.rank_value >= m_max_rank_info.rank_value)
				{
					// 代表从当前排行榜上删除这个玩家
					m_key_to_node.erase(one_player.player_id);
					return 0;
				}
				temp_node->rank_info_ptr->update_value_and_ts(one_player.rank_value, gen_next_update_ts());
			}
		}

		std::array<node *, MAX_LEVEL> prev_nodes;
		std::array<std::uint32_t, MAX_LEVEL> prev_ranks;
		get_prev_nodes(*temp_node, prev_nodes, prev_ranks);
		if (prev_ranks[0] == m_pool_sz)
		{
			m_key_to_node.erase(one_player.player_id);
			return m_pool_sz + 1;
		}
		int cur_node_level = temp_node->m_level;

		// 调整next指针与span
		for (int i = cur_node_level; i >= 0; --i)
		{
			temp_node->nexts[i] = prev_nodes[i]->nexts[i];
			// temp_node->nexts[i] 的全局排名 A
			// A = prev_ranks[i] + prev_nodes[i]->spans[i] + 2;
			// B = prev_ranks[0] + 1 就是当前新节点的全局排名 B
			// A - B -1 就是当前节点到next[i]之间的span数量
			// A - B - 1 = prev_ranks[i] + prev_nodes[i]->spans[i] - prev_ranks[0];
			temp_node->spans[i] = prev_ranks[i] + prev_nodes[i]->spans[i] - prev_ranks[0];
			assert(temp_node->spans[i] < size() + 100);
			// C = prev_ranks[i] 就是当前prev[i] 的全局排名
			// B - C - 1就是prev[i] 的新span[i]
			// B - C - 1 = prev_ranks[0] - prev_ranks[i]
			prev_nodes[i]->spans[i] = prev_ranks[0] - prev_ranks[i];
			assert(prev_nodes[i]->spans[i] < size() + 100);
			prev_nodes[i]->nexts[i] = temp_node;
		}
		for (int i = cur_node_level + 1; i <= m_level; i++)
		{
			prev_nodes[i]->spans[i] += 1;
		}
		if (temp_node->nexts[0] == &m_min_node)
		{
			m_prev_node_for_min = temp_node;
		}
		if (size() > m_pool_sz)
		{
			shrink_to_pool_sz();
		}
		return prev_ranks[0] + 1;
	}

	bool skiplist_rank::remove(const std::string& key)
	{
		auto temp_node = find_node(key);
		if (temp_node == nullptr)
		{
			return false;
		}
		else
		{
			remove_from_list(temp_node);
			m_key_to_node.erase(key);
			return true;
		}
	}

	int skiplist_rank::level() const
	{
		return m_level;
	}

	std::uint32_t skiplist_rank::size() const
	{
		return m_key_to_node.size();
	}

	std::pair<const rank_info*, std::uint32_t> skiplist_rank::get_rank(const std::string &key) const
	{
		auto temp_node = find_node(key);
		if (temp_node == nullptr)
		{
			return std::make_pair<const rank_info*, std::uint32_t>(nullptr, 0);
		}

		std::array<const node *, MAX_LEVEL> prev_nodes;
		std::array<std::uint32_t, MAX_LEVEL> prev_ranks;
		get_prev_nodes_const(*temp_node, prev_nodes, prev_ranks);
		return std::make_pair(temp_node->rank_info_ptr, prev_ranks[0] + 1);
	}

	// 有多个相同value时 返回最后一个value的排名
	std::uint32_t skiplist_rank::get_rank(double value) const
	{
		if (*m_prev_node_for_min < value)
		{
			return size() + 1;
		}
		rank_info temp_rank_info;
		temp_rank_info.rank_value = value;
		temp_rank_info.update_ts = std::numeric_limits<std::uint64_t>::max();

		node temp_node(&temp_rank_info, m_level + 1, false);
		std::array<const node *, MAX_LEVEL> prev_nodes;
		std::array<std::uint32_t, MAX_LEVEL> prev_ranks;
		get_prev_nodes_const(temp_node, prev_nodes, prev_ranks);
		return prev_ranks[0] + 1;
	}

	const rank_info *skiplist_rank::get_player(std::uint32_t in_rank) const
	{
		auto result_node = get_node_for_rank(in_rank);
		if (result_node)
		{
			return result_node->rank_info_ptr;
		}
		else
		{
			return nullptr;
		}
	}
	// 获取[in_begin_rank, in_end_rank]之间的节点
	std::vector<const rank_info *> skiplist_rank::get_players(std::uint32_t in_begin_rank, std::uint32_t in_end_rank) const
	{
		if (in_end_rank > size())
		{
			in_end_rank = size();
		}
		if (in_begin_rank == 0)
		{
			in_begin_rank = 1;
		}
		if (in_begin_rank > in_end_rank)
		{
			return {};
		}
		auto begin_node = get_node_for_rank(in_begin_rank);
		auto end_node = get_node_for_rank(in_end_rank);
		if (!begin_node || !end_node)
		{
			assert(false);
			return {};
		}
		std::vector<const rank_info *> result;
		result.reserve(in_end_rank - in_begin_rank + 1);
		while (begin_node != end_node)
		{
			result.push_back(begin_node->rank_info_ptr);
			begin_node = begin_node->nexts[0];
		}
		result.push_back(end_node->rank_info_ptr);
		return result;
	}
	json skiplist_rank::encode() const
	{
		json result;
		result["min_value"] = m_min_rank_info.rank_value;
		result["max_value"] = m_max_rank_info.rank_value;
		json::array_t all_nodes_json;
		all_nodes_json.reserve(size());
		auto begin_node = m_max_node.nexts[0];
		while (begin_node != &m_min_node)
		{
			all_nodes_json.push_back(*begin_node->rank_info_ptr);
		}
		result["sorted_ranks"] = std::move(all_nodes_json);
		return result;
	}
	bool skiplist_rank::decode(const json &data)
	{
		std::vector<rank_info> temp_rank_infos;
		try
		{
			data.at("sorted_ranks").get_to(temp_rank_infos);
		}
		catch (std::exception &e)
		{
			std::cerr << "fail to decode skiplist rank with exception " << e.what() << std::endl;
			return false;
		}
		for (auto &one_rank_info : temp_rank_infos)
		{
			if (one_rank_info.rank_value <= m_min_rank_info.rank_value || one_rank_info.rank_value >= m_max_rank_info.rank_value)
			{
				continue;
			}
			update(one_rank_info);
		}
		return true;
	}
	skiplist_rank::node *skiplist_rank::get_node_for_rank(std::uint32_t in_rank) const
	{
		if (in_rank == 0 || in_rank > size())
		{
			return nullptr;
		}
		const node *search_node = &m_max_node;
		std::uint32_t last_level_prev_rank = 0;
		for (int i = m_level; i >= 0; --i)
		{

			// 由于我们保证了最末尾有一个m_min_node
			// 所以search_node->next[i]永远存在
			while (last_level_prev_rank < in_rank)
			{
				auto temp_next_rank = last_level_prev_rank + search_node->spans[i] + 1;
				if (temp_next_rank == in_rank)
				{
					return search_node->nexts[i];
				}
				else if (temp_next_rank > in_rank)
				{
					break;
				}
				else
				{
					last_level_prev_rank = temp_next_rank;
					search_node = search_node->nexts[i];
				}
			}
		}
		return nullptr;
	}

	void skiplist_rank::shrink_to_pool_sz()
	{
		auto dest_rank = m_pool_sz + 1;
		auto remove_begin_node = get_node_for_rank(dest_rank);
		if (!remove_begin_node)
		{
			return;
		}
		std::array<node*, MAX_LEVEL> prev_nodes;
		std::array<std::uint32_t, MAX_LEVEL> prev_ranks;
		get_prev_nodes(*remove_begin_node, prev_nodes, prev_ranks);
		
		while (remove_begin_node != &m_min_node)
		{
			auto temp_node = remove_begin_node->nexts[0];
			for (int i = 0; i <= m_level; i++)
			{
				if (prev_nodes[i]->nexts[i] == remove_begin_node)
				{
					prev_nodes[i]->spans[i] += remove_begin_node->spans[i];
					prev_nodes[i]->nexts[i] = remove_begin_node->nexts[i];
				}
				else
				{
					prev_nodes[i]->spans[i] -= 1;
				}
			}
			m_key_to_node.erase(remove_begin_node->rank_info_ptr->player_id);
			remove_begin_node = temp_node;
		}
		m_prev_node_for_min = prev_nodes[0];
	}

	bool skiplist_rank::update_player_info(const std::string& player_id, const json::object_t& player_info)
	{
		auto result_node = find_node(player_id);
		if (result_node)
		{
			result_node->rank_info_ptr->player_info = player_info;
			return true;
		}
		else
		{
			return false;
		}
	}

	void skiplist_rank::debug_print() const
	{
		std::cout << "<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
		for (int i = m_level; i > 0; i--)
		{
			auto temp_next = &m_max_node;
			while (temp_next != &m_min_node)
			{
				std::cout << "(" << temp_next->rank_info_ptr->rank_value << "," << temp_next->spans[i] << ") ";
				temp_next = temp_next->nexts[i];
			}
			std::cout << std::endl;
		}
		std::cout << ">>>>>>>>>>>>>>>>>>>" << std::endl;
	}
}