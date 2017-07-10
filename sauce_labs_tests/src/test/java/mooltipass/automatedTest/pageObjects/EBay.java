package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

//Same Problem The subdomain issue, Mooltipass doesnot know which domain to save pass in 
public class EBay extends AbstractPage{
	
	public EBay (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	
	@FindBy(xpath = "//a[contains(text(),'Sign in')]")
	private WebElement loginBtn;
	
	
	
	@FindBy(xpath = "//input[@placeholder='Email or username'][@class='fld']")
	private WebElement email;
	
	
	@FindBy(xpath = "//input[@placeholder='Password'][@class='fld']")
	private WebElement password;
	@FindBy(id = "sgnBt")
	private WebElement submitLogin;
	

	@FindBy(xpath = "//a[@class='gh-ua gh-control']")
	private WebElement dashBoard;
	@FindBy(xpath = "//a[@href='https://signin.ebay.com/ws/eBayISAPI.dll?SignIn&lgout=1']")
	private WebElement logoutBtn;
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);		
		loginBtn.click();
	}
	
	public void submit(){
	submitLogin.click();
	}
	public void goTodDashboard()
	{
		waitUntilAppears(dashBoard);
		dashBoard.click();
		
	}
	public void logout(){
		logoutBtn.click();
	}
	
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//a[@class='gh-ua gh-control']"));
		return isElementPresent(By.xpath("//a[@class='gh-ua gh-control']"));
	}
	public boolean checkAtLoginPage(){
		//waitUntilAppears(By.id("sgnBt"));
		return isElementPresent(By.id("sgnBt"));
	}
	

}
