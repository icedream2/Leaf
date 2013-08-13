package com.test2;

import java.io.*;

import java.net.URL;
import java.net.URLConnection;

import org.htmlparser.Node;
import org.htmlparser.NodeFilter;
import org.htmlparser.Parser;
import org.htmlparser.filters.AndFilter;
import org.htmlparser.filters.HasAttributeFilter;
import org.htmlparser.filters.NodeClassFilter;
import org.htmlparser.filters.OrFilter;
import org.htmlparser.filters.TagNameFilter;
import org.htmlparser.nodes.TextNode;
import org.htmlparser.tags.LinkTag;
import org.htmlparser.tags.Span;
import org.htmlparser.tags.TitleTag;
import org.htmlparser.util.NodeList;
import org.htmlparser.visitors.HtmlPage;

public class HTMLParserForLeaf {

	/**
	 * @param args
	 * @throws Exception 
	 */
	public static void main(String[] args) throws Exception {
		// TODO Auto-generated method stub
		FileOutputStream out=new FileOutputStream("D:/test.txt");
		PrintStream p=new PrintStream(out);
		for(int i=1;i<101;i++){
			String path="http://www.plantphoto.cn/tu/"+i;
			URL url=new URL(path);		
			URLConnection conn=url.openConnection();
			conn.setDoOutput(true);
			InputStream inputStream=conn.getInputStream();
			InputStreamReader isr=new InputStreamReader(inputStream,"utf-8");
			StringBuffer sb=new StringBuffer();
			BufferedReader in=new BufferedReader(isr);
			String inputLine;
			while((inputLine=in.readLine())!=null){
				sb.append(inputLine);
				sb.append("\n");	
			}
			String result=sb.toString();
			//readByHtml(result);
			p.print("ID: "+i);
			readLeafName(result,p);
			readTitle(result,p);
		}
	}
	public static void readByHtml(String content)throws Exception{
		Parser myParser;
		myParser=Parser.createParser(content, "utf-8");
		HtmlPage visitor=new HtmlPage(myParser);
		myParser.visitAllNodesWith(visitor);
		String testInPage=visitor.getTitle();
		System.out.println(testInPage);
		NodeList nodelist;
		nodelist=visitor.getBody();
		System.out.println(nodelist.asString().trim());
	}
	public static void readLeafName(String result,PrintStream path)throws Exception{
		Parser parser = Parser.createParser(result, "utf-8"); 
        AndFilter filter = 
          new AndFilter( 
                        new TagNameFilter("span"), 
                       new HasAttributeFilter("id","Label15") 
        ); 
        NodeList n = parser.parse(filter); 
        Node[] nodes=n.toNodeArray();
		String line="";
		for(int i=0;i<nodes.length;i++){
			Node node=nodes[i];
			Span s=(Span)node;
			line=s.getChildrenHTML();
			line=getName(line);
			if(isTrimEmpty(line))
				continue;
			//System.out.println(line);
			path.print("中文名：");
			path.print(line);
		}
	}
	public static String getName(String child){
		String str="";
		for(int i=0;i<child.length();i++){
			//System.out.println(child.charAt(i));
			if(child.charAt(i)=='>'|| child.charAt(i)=='<' ||child.charAt(i)=='b')
				continue;
			if(child.charAt(i)=='/' && child.charAt(i+1)=='b')
				break;
			str+=child.charAt(i);
		}
		return str;
	}
	public static void readTitle(String result,PrintStream path)throws Exception{
		Parser parser;
		NodeList nodelist;
		parser=Parser.createParser(result, "utf-8");
		NodeFilter titleFilter =new NodeClassFilter(TitleTag.class);
		OrFilter lastFilter=new OrFilter();
		lastFilter.setPredicates(new NodeFilter[]{titleFilter});
		nodelist=parser.parse(lastFilter);
		Node[] nodes=nodelist.toNodeArray();
   
		String line="";
		for(int i=0;i<nodes.length;i++){
			Node node=nodes[i];
			TitleTag titlenode=(TitleTag)node;
			line=titlenode.getTitle();
			line=dealChinese(line);
			if(isTrimEmpty(line))
				continue;
			path.print("  英文名：");
			path.println(line);
		}
	}
	public static String dealChinese(String line){
		String str="";
		//str =line.replaceAll("[^x00-xff]*", "");
		//str =line.replaceAll("[^x00-xff]*", "");
		str = line.replaceAll("(\\s[\u4E00-\u9FA5]+)|([\u4E00-\u9FA5]+\\s)", "");
		return str;
	}
	public static void readTextAndLinkAndTitle(String result) throws Exception {
		Parser parser;
		NodeList nodelist;
		parser=Parser.createParser(result, "utf-8");
		NodeFilter textFilter=new NodeClassFilter(TextNode.class);
		NodeFilter linkFilter=new NodeClassFilter(LinkTag.class);
		NodeFilter titleFilter =new NodeClassFilter(TitleTag.class);
		OrFilter lastFilter=new OrFilter();
		lastFilter.setPredicates(new NodeFilter[]{titleFilter});
		nodelist=parser.parse(lastFilter);
		Node[] nodes=nodelist.toNodeArray();
   
		String line="";
		for(int i=0;i<nodes.length;i++){
			Node node=nodes[i];
			if(node instanceof TextNode){
				TextNode textnode=(TextNode)node;
				line=textnode.getText();
			}else if(node instanceof LinkTag){
				LinkTag link=(LinkTag)node;
				line=link.getLink();
			}else if(node instanceof TitleTag){
				TitleTag titlenode=(TitleTag)node;
				line=titlenode.getTitle();
			}

			if(isTrimEmpty(line))
				continue;
			System.out.println(line);
		}
	}
	public static boolean isTrimEmpty(String astr){
		if((null==astr)||(astr.length()==0)){
			return true;
		}
		if(isBlank(astr.trim())){
			return true;
		}
		return false;
	}
	public static boolean isBlank(String astr){
		if((null==astr)||(astr.length()==0)){
			return true;
		}else{
			return false;
		}
	}
}
