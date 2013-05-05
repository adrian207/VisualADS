package cnd.conflict.detect;

import java.util.ArrayList;
import java.util.List;

import cnd.conflict.dao.PolicyDAO;
import cnd.conflict.entity.CndPolicy;
import cnd.conflict.entity.Result;
import cnd.conflict.frame.Frame;

/**
 * @author YC
 * 
 */
public class ConflictAnalysis {

	/**
	 * @param args
	 */
	private static final int UNRELATION = 0;// �޳�ͻ
	private static final int SYNTAXCONFLICT = 1;// �﷨��ͻ
	private static final int INCLUSIVEMATCH = 2;// ������ͻ
	private static final int PARTIALMATCH = 3;// ��س�ͻ
	private ElementRelation elementRelation = new ElementRelation();
	private static ConflictAnalysis conflictAnalysis = null;

	private String fileActPos = null;
	// ��ʾ�������Եİ����������ڹ�ϵ��0Ϊ������ǰ�߰������ߣ��� 1Ϊ������, 2Ϊ���
	private int fileRoleInc = 0;
	private int fileActInc = 0;

	private PolicyDAO policyDAO = new PolicyDAO();

	public PolicyDAO getPolicyDAO() {
		return policyDAO;
	}

	// private Frame frame = new Frame();
	public int judge_Relation_Policy(CndPolicy policy1, CndPolicy policy2) {
		// TODO �жϲ��Թ�ϵ
		List<Integer> iList = new ArrayList<Integer>();
		if (policy1.getContext().equals(policy2.getContext())
				&& policy1.getRole().equals(policy2.getRole())
				&& policy1.getView().equals(policy2.getView())
				&& policy1.getActivity().equals(policy2.getActivity())) {
			return SYNTAXCONFLICT;
		}
		iList.add(elementRelation.judge_Relation_Context(policy1.getContext(),
				policy2.getContext()));
		iList.add(elementRelation.judge_Relation_Role(policy1.getRole(),
				policy2.getRole()));
		iList.add(elementRelation.judge_Relation_View(policy1.getView(),
				policy2.getView()));
		iList.add(elementRelation.judge_Relation_Activity(
				policy1.getActivity(), policy2.getActivity()));
		if (iList.contains(0)) {
			return UNRELATION;
		} else {
			for (Integer element : iList) {
				if (iList.contains(1)) {
					return PARTIALMATCH;
				}
			}
			if (iList.contains(3) && iList.contains(4)) {
				return PARTIALMATCH;
			} else {
				return INCLUSIVEMATCH;
			}
		}

	}

	// �ļ����ʿ����жϹ�ϵ
	public int judge_Relation_Policy(CndPolicy policy1, CndPolicy policy2,
			String dataBase) {

		// TODO �жϲ��Թ�ϵ
		List<Integer> iList = new ArrayList<Integer>();
		/*
		 * if (policy1.getOrganization().equals(policy2.getOrganization()) &&
		 * policy1.getContext().equals(policy2.getContext()) &&
		 * policy1.getRole().equals(policy2.getRole()) &&
		 * policy1.getView().equals(policy2.getView()) &&
		 * policy1.getActivity().equals(policy2.getActivity())) { return
		 * SYNTAXCONFLICT; }
		 */
		// �����ж���֯ ������ �ļ��� ��һ����϶��޹� ��ֻ���ڰ������޹�������� ע��Ψһ��ʶ
		// ��֯
		iList.add(elementRelation.judge_Relation_Organization(policy1
				.getOrganization(), policy2.getOrganization(), dataBase));
		// ������
		iList.add(elementRelation.judge_Relation_Context(policy1
				.getOrganization()
				+ "\\" + policy1.getContext(), policy2.getOrganization() + "\\"
				+ policy2.getContext(), dataBase));
		// �ļ���
		iList.add(elementRelation.judge_Relation_View(policy1.getOrganization()
				+ "\\" + policy1.getContext() + "\\" + policy1.getView(),
				policy2.getOrganization() + "\\" + policy2.getContext() + "\\"
						+ policy2.getView(), dataBase));
		// ��ɫ
		iList.add(elementRelation.judge_Relation_Role(policy1.getRole(),
				policy2.getRole(), dataBase));
		// �
		iList.add(elementRelation.judge_Relation_Activity(
				policy1.getActivity(), policy2.getActivity(), dataBase));

		if (iList.contains(0)) {
			return UNRELATION;
		} else {
			for (Integer element : iList) {
				if (iList.contains(1)) {
					return PARTIALMATCH;
				}
			}
			if (iList.contains(3) && iList.contains(4)) {
				return PARTIALMATCH;
			} else {
				return INCLUSIVEMATCH;
			}
		}
	}

	public String conflict_Analysis(Frame frame, String dataBase) {
		return new ShowConflictThread(frame, dataBase, this).start();
	}

	public String conflict_Analysis(Frame frame) {
		// TODO��ͻ����,���д���ͻ����
		CndPolicy policy1 = null, policy2 = null;
		Result result = new Result();
		int numberOfPolicy = policyDAO.getPolicyAmount();
		StringBuffer strTotal = new StringBuffer();
		int policyAmount = numberOfPolicy;
		int conflictAmount = 0;
		int unralation = 0;
		int syntaxConflict = 0;
		int inclusiveConflict = 0;
		int partialConflict = 0;

		for (int i = 1; i < numberOfPolicy; i++) {
			for (int j = i + 1; j <= numberOfPolicy; j++) {
				policy1 = policyDAO.getOnePolicy(i);
				policy2 = policyDAO.getOnePolicy(j);
				String str = null;
				String str_Policy1 = "policy" + policy1.getPolicyId();
				String str_Policy2 = "policy" + policy2.getPolicyId();
				if (policyDAO.getMeasureElementDenyList(policy1.getMeasure())
						.contains(policy2.getMeasure())) {
					if (judge_Relation_Policy(policy1, policy2) == 0) {
						str = "�޳�ͻ";
						unralation++;
					}
					if (judge_Relation_Policy(policy1, policy2) == 1) {
						str = "�﷨��ͻ";
						StringBuffer strSyntax = new StringBuffer();
						strSyntax.append("\n");
						strSyntax.append("��ͻ����");
						strSyntax.append("\n");
						strSyntax.append(str_Policy1 + " " + str_Policy2);
						strSyntax.append("\n");
						strSyntax.append("��ͻλ�ã�");
						strSyntax.append("\n");
						strSyntax.append("all policy elements");
						strSyntax.append("\n");
						strSyntax.append("��ͻԭ��");
						strSyntax.append("\n");
						strSyntax.append("systax conflict");
						str = str + strSyntax.toString();
						syntaxConflict++;
					}
					if (judge_Relation_Policy(policy1, policy2) == 2) {
						str = "������ͻ";
						StringBuffer strSyntax = new StringBuffer();
						strSyntax.append("\n");
						strSyntax.append("��ͻ����");
						strSyntax.append("\n");
						strSyntax.append(str_Policy1 + " " + str_Policy2);
						strSyntax.append("\n");
						strSyntax.append("��ͻλ�ã�");
						strSyntax.append("\n");
						strSyntax.append("all policy elements");
						strSyntax.append("\n");
						strSyntax.append("��ͻԭ��");
						strSyntax.append("\n");
						strSyntax.append("inclusive conflict");
						str = str + strSyntax.toString();

						inclusiveConflict++;
					}
					if (judge_Relation_Policy(policy1, policy2) == 3) {
						str = "��س�ͻ";
						partialConflict++;
						StringBuffer strTemp = new StringBuffer();
						strTemp.append("\n");
						strTemp.append("��ͻ����");
						strTemp.append("\n");
						strTemp.append(str_Policy1 + " " + str_Policy2);
						strTemp.append("\n");
						strTemp.append("��ͻλ�ü�ԭ��:");
						if (elementRelation.judge_Relation_Context(policy1
								.getContext(), policy2.getContext()) == 1) {
							strTemp.append("Context ���ڽ���");
							strTemp.append(",");
							// strTemp.append("\n");
						}
						if (elementRelation.judge_Relation_Role(policy1
								.getRole(), policy2.getRole()) == 1) {
							strTemp.append("Role ���ڽ���");
							strTemp.append(",");
							// strTemp.append("\n");
						}
						if (elementRelation.judge_Relation_View(policy1
								.getView(), policy2.getView()) == 1) {
							strTemp.append("View ���ڽ���");
							strTemp.append(",");
							// strTemp.append("\n");
						}
						if (elementRelation.judge_Relation_Activity(policy1
								.getActivity(), policy2.getActivity()) == 1) {
							strTemp.append("Activity ���ڽ���");
							strTemp.append(",");
							// strTemp.append("\n");
						}

						str = str + strTemp.toString();

					}

				} else {
					str = "�޳�ͻ";
					unralation++;
				}
				StringBuffer strBuffer = new StringBuffer();
				strBuffer.append(str_Policy1);
				strBuffer.append(" and ");
				strBuffer.append(str_Policy2);
				strBuffer.append(" exsit ");
				strBuffer.append(str);
				// frame.getJTextArea().setText(strBuffer.toString());
				System.out.println(strBuffer.toString());

				strTotal.append(strBuffer.toString());
				strTotal.append("\n");

			}
		}
		conflictAmount = syntaxConflict + inclusiveConflict + partialConflict;
		result.setPolicyAmount(policyAmount);
		result.setConflictAmount(conflictAmount);
		result.setUnrelation(unralation);
		result.setSyntaxconflict(syntaxConflict);
		result.setInclusivematch(inclusiveConflict);
		result.setPartialmatch(partialConflict);
		// �������������ͬ�����������
		if (!policyDAO.getPolicyAmountList().contains(policyAmount)) {
			policyDAO.addResult(result);
		}

		frame.getMessageArea().setText(strTotal.toString());
		return strTotal.toString();

	}

	public String conflict_Analysis_English(Frame frame, String dataBase) {
		CndPolicy policy1 = null, policy2 = null;
		Result result = new Result();
		int numberOfPolicy = policyDAO.getPolicyAmount();
		StringBuffer strTotal = new StringBuffer();
		int policyAmount = numberOfPolicy;
		int conflictAmount = 0;
		int unralation = 0;
		int syntaxConflict = 0;
		int inclusiveConflict = 0;
		int partialConflict = 0;

		for (int i = 1; i < numberOfPolicy; i++) {
			for (int j = i + 1; j <= numberOfPolicy; j++) {
				policy1 = policyDAO.getOnePolicy(i);
				policy2 = policyDAO.getOnePolicy(j);
				String str = null;
				String str_Policy1 = "policy" + policy1.getPolicyId();
				String str_Policy2 = "policy" + policy2.getPolicyId();
				if (!(policy1.getMeasure().equals(policy2.getMeasure()))) {
					if (judge_Relation_Policy(policy1, policy2, dataBase) == 0) {
						str = "No conflict";
						unralation++;
					} else {
						/*
						 * if (judge_Relation_Policy(policy1, policy2, dataBase)
						 * == 1) { str = "�﷨��ͻ"; StringBuffer strSyntax = new
						 * StringBuffer(); strSyntax.append("\n");
						 * strSyntax.append("��ͻ����"); strSyntax.append("\n");
						 * strSyntax.append(str_Policy1 + " " + str_Policy2);
						 * strSyntax.append("\n"); strSyntax.append("��ͻλ�ã�");
						 * strSyntax.append("\n");
						 * strSyntax.append("all policy elements");
						 * strSyntax.append("\n"); strSyntax.append("��ͻԭ��");
						 * strSyntax.append("\n");
						 * strSyntax.append("systax conflict"); str = str +
						 * strSyntax.toString(); syntaxConflict++; }
						 */
						if (judge_Relation_Policy(policy1, policy2, dataBase) == 2) {
							str = "Inclusion Conflict";
							StringBuffer strSyntax = new StringBuffer();
							strSyntax.append("\n");
							strSyntax.append("Conflict Policy��");
							strSyntax.append("\n");
							strSyntax.append(str_Policy1 + " " + str_Policy2);
							strSyntax.append("\n");
							strSyntax.append("Conflict Position��");
							strSyntax.append("\n");
							strSyntax.append(str_Policy1 + "_"
									+ policy1.getRole()
									+ " have conflict with " + str_Policy2
									+ "_" + policy2.getRole() + " to folder "
									+ policy1.getOrganization() + "\\"
									+ policy1.getContext() + "\\"
									+ policy1.getView() + " with action of "
									+ this.fileActPos + "");
							strSyntax.append("\n");
							strSyntax.append("Conflict Reason��");
							strSyntax.append("\n");
							if (0 == this.getFileRoleInc()) {
								strSyntax.append(str_Policy2 + "'s role "
										+ policy2.getRole() + " inherits "
										+ str_Policy1 + "'s role "
										+ policy1.getRole());
							} else if (1 == this.getFileRoleInc()) {
								strSyntax.append(str_Policy1 + "'s role "
										+ policy1.getRole() + " inherits "
										+ str_Policy2 + "'s role"
										+ policy2.getRole());
							} else if (3 == this.getFileRoleInc()) {

							} else {
								strSyntax.append(str_Policy1 + "'s role "
										+ policy1.getRole()
										+ " is the same with " + str_Policy2
										+ "'s role " + policy2.getRole() + "");
							}
							strSyntax.append("\n");
							strSyntax.append(str_Policy1 + "'s activity "
									+ policy1.getActivity() + " and "
									+ str_Policy2 + " 's activity "
									+ policy2.getActivity()
									+ " have contain relation " + "("
									+ this.fileActPos + ")");
							strSyntax.append("\n");
							if (0 == this.getFileActInc()) {
								strSyntax.append(str_Policy1 + "'s activity "
										+ policy1.getActivity() + " contains "
										+ str_Policy2 + "'s activity");
							} else if (1 == this.getFileActInc()) {
								strSyntax.append(str_Policy1 + "'s activity "
										+ policy1.getActivity()
										+ "is contained within " + str_Policy2
										+ "'s activity");
							} else if (3 == this.getFileActInc()) {

							} else {
								strSyntax.append(str_Policy1 + "'s activity "
										+ policy1.getActivity()
										+ " is the same with " + str_Policy2
										+ "'s activity");
							}

							strSyntax.append("\n");
							str = str + strSyntax.toString();

							inclusiveConflict++;
						}
						if (judge_Relation_Policy(policy1, policy2, dataBase) == 3) {
							str = "Relevant Conflict";
							partialConflict++;
							StringBuffer strTemp = new StringBuffer();
							strTemp.append("\n");
							strTemp.append("Conflict Policy��");
							strTemp.append("\n");
							strTemp.append(str_Policy1 + " " + str_Policy2);
							strTemp.append("\n");
							strTemp.append("Conflict Position:");
							strTemp.append("\n");
							strTemp.append(str_Policy1 + "_"
									+ policy1.getRole()
									+ " have conflict with " + str_Policy2
									+ "_" + policy2.getRole() + " to folder "
									+ policy1.getOrganization() + "\\"
									+ policy1.getContext() + "\\"
									+ policy1.getView() + " with action of "
									+ this.fileActPos + "");
							strTemp.append("\n");
							strTemp.append("Conflict Reason��");
							strTemp.append("\n");
							if (0 == this.getFileRoleInc()) {
								strTemp.append(str_Policy2 + "'s role "
										+ policy2.getRole() + " inherits "
										+ str_Policy1 + "'s role "
										+ policy1.getRole());
							} else if (1 == this.getFileRoleInc()) {
								strTemp.append(str_Policy1 + "'s role "
										+ policy1.getRole() + " inherits "
										+ str_Policy2 + "'s role "
										+ policy2.getRole());
							} else if (3 == this.getFileRoleInc()) {
								strTemp.append(str_Policy1 + "'s group "
										+ policy1.getRole() + " and "
										+ str_Policy2 + "'s group "
										+ policy2.getRole()
										+ " have the same user");
							} else {
								strTemp.append(str_Policy1 + "'s role "
										+ policy1.getRole()
										+ " is the same with " + str_Policy2
										+ "'s role " + policy2.getRole() + "");
							}
							strTemp.append("\n");
							strTemp.append(str_Policy1 + "'s activity "
									+ policy1.getActivity() + " and "
									+ str_Policy2 + "'s activity "
									+ policy2.getActivity()
									+ " have contain relation " + "("
									+ this.fileActPos + ")");
							strTemp.append("\n");
							// strTemp.append("\n");

							str = str + strTemp.toString();

						}
						StringBuffer strBuffer = new StringBuffer();
						strBuffer.append(str_Policy1);
						strBuffer.append(" and ");
						strBuffer.append(str_Policy2);
						strBuffer.append(" exsit ");
						strBuffer.append(str);
						// frame.getJTextArea().setText(strBuffer.toString());
						System.out.println(strBuffer.toString());

						strTotal.append(strBuffer.toString());
						strTotal.append("\n");
					}
				}
			}
		}
		conflictAmount = syntaxConflict + inclusiveConflict + partialConflict;
		result.setPolicyAmount(policyAmount);
		result.setConflictAmount(conflictAmount);
		result.setUnrelation(unralation);
		// result.setSyntaxconflict(syntaxConflict);
		result.setInclusivematch(inclusiveConflict);
		result.setPartialmatch(partialConflict);
		// �������������ͬ�����������
		if (!policyDAO.getPolicyAmountList().contains(policyAmount)) {
			policyDAO.addResult(result);
		}

		frame.getMessageArea().setText(strTotal.toString());

		return strTotal.toString();
	}

	public String conflict_Analysis_English(Frame frame) {
		// TODO��ͻ����,���д���ͻ����
		CndPolicy policy1 = null, policy2 = null;
		Result result = new Result();
		int numberOfPolicy = policyDAO.getPolicyAmount();
		StringBuffer strTotal = new StringBuffer();
		int policyAmount = numberOfPolicy;
		int conflictAmount = 0;
		int unralation = 0;
		int syntaxConflict = 0;
		int inclusiveConflict = 0;
		int partialConflict = 0;

		for (int i = 1; i < numberOfPolicy; i++) {
			for (int j = i + 1; j <= numberOfPolicy; j++) {
				policy1 = policyDAO.getOnePolicy(i);
				policy2 = policyDAO.getOnePolicy(j);
				String str = null;
				String str_Policy1 = "policy" + policy1.getPolicyId();
				String str_Policy2 = "policy" + policy2.getPolicyId();
				if (policyDAO.getMeasureElementDenyList(policy1.getMeasure())
						.contains(policy2.getMeasure())) {
					if (judge_Relation_Policy(policy1, policy2) == 0) {
						str = "No conflict";
						unralation++;
					}
					if (judge_Relation_Policy(policy1, policy2) == 1) {
						str = "Grammar Conflict";
						StringBuffer strSyntax = new StringBuffer();
						strSyntax.append("\n");
						strSyntax.append("Conflict policy��");
						strSyntax.append("\n");
						strSyntax.append(str_Policy1 + " " + str_Policy2);
						strSyntax.append("\n");
						strSyntax.append("Conflict Position��");
						strSyntax.append("\n");
						strSyntax.append("all policy elements");
						strSyntax.append("\n");
						strSyntax.append("Conflict Reason��");
						strSyntax.append("\n");
						strSyntax.append("systax conflict");
						str = str + strSyntax.toString();
						syntaxConflict++;
					}
					if (judge_Relation_Policy(policy1, policy2) == 2) {
						str = "Inclusion Conflict";
						StringBuffer strSyntax = new StringBuffer();
						strSyntax.append("\n");
						strSyntax.append("Conflict Policy��");
						strSyntax.append("\n");
						strSyntax.append(str_Policy1 + " " + str_Policy2);
						strSyntax.append("\n");
						strSyntax.append("Conflict Position��");
						strSyntax.append("\n");
						strSyntax.append("all policy elements");
						strSyntax.append("\n");
						strSyntax.append("Conflict Reason��");
						strSyntax.append("\n");
						strSyntax.append("inclusive conflict");
						str = str + strSyntax.toString();

						inclusiveConflict++;
					}
					if (judge_Relation_Policy(policy1, policy2) == 3) {
						str = "Revelant Conflict";
						partialConflict++;
						StringBuffer strTemp = new StringBuffer();
						strTemp.append("\n");
						strTemp.append("Conflict Policy��");
						strTemp.append("\n");
						strTemp.append(str_Policy1 + " " + str_Policy2);
						strTemp.append("\n");
						strTemp.append("Conflict Position & Reason:");
						if (elementRelation.judge_Relation_Context(policy1
								.getContext(), policy2.getContext()) == 1) {
							strTemp.append("Context have overlapped");
							strTemp.append(",");
							// strTemp.append("\n");
						}
						if (elementRelation.judge_Relation_Role(policy1
								.getRole(), policy2.getRole()) == 1) {
							strTemp.append("Role have overlapped");
							strTemp.append(",");
							// strTemp.append("\n");
						}
						if (elementRelation.judge_Relation_View(policy1
								.getView(), policy2.getView()) == 1) {
							strTemp.append("View have overlapped");
							strTemp.append(",");
							// strTemp.append("\n");
						}
						if (elementRelation.judge_Relation_Activity(policy1
								.getActivity(), policy2.getActivity()) == 1) {
							strTemp.append("Activity have overlapped");
							strTemp.append(",");
							// strTemp.append("\n");
						}

						str = str + strTemp.toString();

					}

				} else {
					str = "No conflict";
					unralation++;
				}
				StringBuffer strBuffer = new StringBuffer();
				strBuffer.append(str_Policy1);
				strBuffer.append(" and ");
				strBuffer.append(str_Policy2);
				strBuffer.append(" exsit ");
				strBuffer.append(str);
				// frame.getJTextArea().setText(strBuffer.toString());
				System.out.println(strBuffer.toString());

				strTotal.append(strBuffer.toString());
				strTotal.append("\n");

			}
		}
		conflictAmount = syntaxConflict + inclusiveConflict + partialConflict;
		result.setPolicyAmount(policyAmount);
		result.setConflictAmount(conflictAmount);
		result.setUnrelation(unralation);
		result.setSyntaxconflict(syntaxConflict);
		result.setInclusivematch(inclusiveConflict);
		result.setPartialmatch(partialConflict);
		// �������������ͬ�����������
		if (!policyDAO.getPolicyAmountList().contains(policyAmount)) {
			policyDAO.addResult(result);
		}

		frame.getMessageArea().setText(strTotal.toString());
		return strTotal.toString();

	}

	/*
	 * private ConflictAnalysis(){
	 * 
	 * }
	 */
	/**
	 * ��ȡ��̬ʵ��
	 * 
	 * @return RelationGraph
	 */
	public static ConflictAnalysis getInstance() {
		if (conflictAnalysis == null) {
			conflictAnalysis = new ConflictAnalysis();
		}
		return conflictAnalysis;
	}

	/*
	 * public static void main(String[] args) { // TODO Auto-generated method
	 * stub
	 * 
	 * new ConflictAnalysis().test();
	 * 
	 * }
	 */

	public void setFileActPos(String fileActPos) {
		this.fileActPos = fileActPos;
	}

	public String getFileActPos() {
		return fileActPos;
	}

	public void setFileRoleInc(int fileRoleInc) {
		this.fileRoleInc = fileRoleInc;
	}

	public int getFileRoleInc() {
		return fileRoleInc;
	}

	public void setFileActInc(int fileActInc) {
		this.fileActInc = fileActInc;
	}

	public int getFileActInc() {
		return fileActInc;
	}

}

class ShowConflictThread implements Runnable {
	private Thread runner;
	private ConflictAnalysis conAna;
	private String dataBase;
	private Frame frame;
	private String s;

	public ShowConflictThread(Frame frame, String dataBase,
			ConflictAnalysis conAna) {
		// TODO Auto-generated constructor stub
		this.dataBase = dataBase;
		this.conAna = conAna;
		this.frame = frame;
	}

	public String start() {
		// TODO Auto-generated method stub
		runner = new Thread(this);
		runner.start();
		return s;
	}

	@Override
	public void run() {
		// TODO Auto-generated method stub
		String analysisInfo = "Start to detect the conflict...";
		this.frame.getMessageArea().setText(analysisInfo + "\n");
		CndPolicy policy1 = null, policy2 = null;
		Result result = new Result();
		int numberOfPolicy = conAna.getPolicyDAO().getPolicyAmount();
		StringBuffer strTotal = new StringBuffer();
		int policyAmount = numberOfPolicy;
		int conflictAmount = 0;
		int unralation = 0;
		int syntaxConflict = 0;
		int inclusiveConflict = 0;
		int partialConflict = 0;

		for (int i = 1; i < numberOfPolicy; i++) {
			for (int j = i + 1; j <= numberOfPolicy; j++) {
				policy1 = conAna.getPolicyDAO().getOnePolicy(i);
				policy2 = conAna.getPolicyDAO().getOnePolicy(j);
				String str = null;
				String str_Policy1 = "policy" + policy1.getPolicyId();
				String str_Policy2 = "policy" + policy2.getPolicyId();
				if (!(policy1.getMeasure().equals(policy2.getMeasure()))) {
					if (conAna
							.judge_Relation_Policy(policy1, policy2, dataBase) == 0) {
						str = "�޳�ͻ";
						unralation++;
					} else {
						/*
						 * if (judge_Relation_Policy(policy1, policy2, dataBase)
						 * == 1) { str = "�﷨��ͻ"; StringBuffer strSyntax = new
						 * StringBuffer(); strSyntax.append("\n");
						 * strSyntax.append("��ͻ����"); strSyntax.append("\n");
						 * strSyntax.append(str_Policy1 + " " + str_Policy2);
						 * strSyntax.append("\n"); strSyntax.append("��ͻλ�ã�");
						 * strSyntax.append("\n");
						 * strSyntax.append("all policy elements");
						 * strSyntax.append("\n"); strSyntax.append("��ͻԭ��");
						 * strSyntax.append("\n");
						 * strSyntax.append("systax conflict"); str = str +
						 * strSyntax.toString(); syntaxConflict++; }
						 */
						if (conAna.judge_Relation_Policy(policy1, policy2,
								dataBase) == 2) {
							str = "������ͻ";
							StringBuffer strSyntax = new StringBuffer();
							strSyntax.append("\n");
							strSyntax.append("��ͻ����");
							strSyntax.append("\n");
							strSyntax.append(str_Policy1 + " " + str_Policy2);
							strSyntax.append("\n");
							strSyntax.append("��ͻλ�ã�");
							strSyntax.append("\n");
							strSyntax.append(str_Policy1 + "_"
									+ policy1.getRole() + "��" + str_Policy2
									+ "_" + policy2.getRole() + "�ڶ��ļ���"
									+ policy1.getOrganization() + "\\"
									+ policy1.getContext() + "\\"
									+ policy1.getView() + "��"
									+ conAna.getFileActPos() + "��з�����ͻ");
							strSyntax.append("\n");
							strSyntax.append("��ͻԭ��");
							strSyntax.append("\n");
							if (0 == conAna.getFileRoleInc()) {
								strSyntax.append(str_Policy2 + "�Ľ�ɫ"
										+ policy2.getRole() + "�̳���"
										+ str_Policy1 + "�Ľ�ɫ"
										+ policy1.getRole());
							} else if (1 == conAna.getFileRoleInc()) {
								strSyntax.append(str_Policy1 + "�Ľ�ɫ"
										+ policy1.getRole() + "�̳���"
										+ str_Policy2 + "�Ľ�ɫ"
										+ policy2.getRole());
							} else if (3 == conAna.getFileRoleInc()) {

							} else {
								strSyntax.append(str_Policy1 + "�Ľ�ɫ"
										+ policy1.getRole() + "��" + str_Policy2
										+ "�Ľ�ɫ" + policy2.getRole() + "��ͬ");
							}
							strSyntax.append("\n");
							strSyntax.append(str_Policy1 + "�Ļ"
									+ policy1.getActivity() + "��" + str_Policy2
									+ "�Ļ" + policy2.getActivity() + "���ڰ�����ϵ"
									+ "(" + conAna.getFileActPos() + ")");
							strSyntax.append("\n");
							if (0 == conAna.getFileActInc()) {
								strSyntax.append(str_Policy1 + "�Ļ"
										+ policy1.getActivity() + "����"
										+ str_Policy2 + "�Ļ");
							} else if (1 == conAna.getFileActInc()) {
								strSyntax.append(str_Policy1 + "�Ļ"
										+ policy1.getActivity() + "������"
										+ str_Policy2 + "�Ļ");
							} else if (3 == conAna.getFileActInc()) {

							} else {
								strSyntax.append(str_Policy1 + "�Ļ"
										+ policy1.getActivity() + "��"
										+ str_Policy2 + "�Ļ��ͬ");
							}

							strSyntax.append("\n");
							str = strSyntax.toString();

							analysisInfo = str;
							this.frame.getMessageArea().append(
									analysisInfo + "\n");

							inclusiveConflict++;
						}
						if (conAna.judge_Relation_Policy(policy1, policy2,
								dataBase) == 3) {
							str = "��س�ͻ";
							partialConflict++;
							StringBuffer strTemp = new StringBuffer();
							strTemp.append("\n");
							strTemp.append("��ͻ����");
							strTemp.append("\n");
							strTemp.append(str_Policy1 + " " + str_Policy2);
							strTemp.append("\n");
							strTemp.append("��ͻλ��:");
							strTemp.append("\n");
							strTemp.append(str_Policy1 + "_"
									+ policy1.getRole() + "��" + str_Policy2
									+ "_" + policy2.getRole() + "�ڶ��ļ���"
									+ policy1.getOrganization() + "\\"
									+ policy1.getContext() + "\\"
									+ policy1.getView() + "��"
									+ conAna.getFileActPos() + "��з�����ͻ");
							strTemp.append("\n");
							strTemp.append("��ͻԭ��");
							strTemp.append("\n");
							if (0 == conAna.getFileRoleInc()) {
								strTemp.append(str_Policy2 + "�Ľ�ɫ"
										+ policy2.getRole() + "�̳���"
										+ str_Policy1 + "�Ľ�ɫ"
										+ policy1.getRole());
							} else if (1 == conAna.getFileRoleInc()) {
								strTemp.append(str_Policy1 + "�Ľ�ɫ"
										+ policy1.getRole() + "�̳���"
										+ str_Policy2 + "�Ľ�ɫ"
										+ policy2.getRole());
							} else if (3 == conAna.getFileRoleInc()) {
								strTemp.append(str_Policy1 + "����"
										+ policy1.getRole() + "��" + str_Policy2
										+ "����" + policy2.getRole() + "������ͬ���û�");
							} else {
								strTemp.append(str_Policy1 + "�Ľ�ɫ"
										+ policy1.getRole() + "��" + str_Policy2
										+ "�Ľ�ɫ" + policy2.getRole() + "��ͬ");
							}
							strTemp.append("\n");
							strTemp.append(str_Policy1 + "�Ļ"
									+ policy1.getActivity() + "��" + str_Policy2
									+ "�Ļ" + policy2.getActivity() + "���ڽ���"
									+ "(" + conAna.getFileActPos() + ")");
							strTemp.append("\n");
							// strTemp.append("\n");

							str = strTemp.toString();

							analysisInfo = str;
							this.frame.getMessageArea().append(
									analysisInfo + "\n");

						}
						StringBuffer strBuffer = new StringBuffer();
						strBuffer.append(str_Policy1);
						strBuffer.append(" and ");
						strBuffer.append(str_Policy2);
						strBuffer.append(" exsit ");
						strBuffer.append(str);
						// frame.getJTextArea().setText(strBuffer.toString());
						System.out.println(strBuffer.toString());

						strTotal.append(strBuffer.toString());
						strTotal.append("\n");
					}
				}
			}
		}
		conflictAmount = syntaxConflict + inclusiveConflict + partialConflict;
		result.setPolicyAmount(policyAmount);
		result.setConflictAmount(conflictAmount);
		result.setUnrelation(unralation);
		// result.setSyntaxconflict(syntaxConflict);
		result.setInclusivematch(inclusiveConflict);
		result.setPartialmatch(partialConflict);
		// �������������ͬ�����������
		if (!conAna.getPolicyDAO().getPolicyAmountList().contains(policyAmount)) {
			conAna.getPolicyDAO().addResult(result);
		}

		
		//frame.getMessageArea().setText(strTotal.toString());

		this.s = strTotal.toString();

	}

}
