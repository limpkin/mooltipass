package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;
public class AirBnB extends AbstractPage{
	
	public AirBnB (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	private boolean runMobile = false;
	@FindBy(id = "email-login-email")
	private WebElement email;

	@FindBy(id = "email-login-password")
	private WebElement password;
			
	@FindBy(xpath = "//button[@type='submit']//span[text()='Log in']")
	private WebElement submitLogin;
	

	@FindBy(xpath = "//a[@href='/login']")
	private WebElement loginBtn;

	@FindBy(xpath = "//a[@href='/dashboard']")
	private WebElement dashBoard;
	
	@FindBy(xpath = "//button//div[@class='menuIndicator_t7nb5l']")
	private WebElement dashBoardMobile;
	
	@FindBy(xpath = "//a[@href='/logout']")
	private WebElement logoutBtn;

	@FindBy(xpath = "//a[@class='container_ov54o4-o_O-container_notBlock_1xdomts-o_O-container_sizeRegular_1t4vkpr-o_O-container_z4957l']")
	private WebElement logoutApprove;

	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		if(waitUntilAppears(loginBtn))
			loginBtn.click();
		else
		{
			if(waitUntilAppears(dashBoardMobile)){
			clickWithJS(dashBoardMobile);
			loginBtn.click();
			runMobile = true;
			}
		}
	}
	
	public void submit(){
	submitLogin.click();
	}
	public void goTodDashboard()
	{
		if(!runMobile)
		{	
			dashBoard.click();
		}
		else
		{
			runMobile = true;
			clickWithJS(dashBoardMobile);
		}
		
		
	}
	public void logout(){
		waitUntilAppears(logoutBtn);
		logoutBtn.click();
	}
	public void approveLogout()
	{
		if(waitUntilAppears(logoutApprove))
			logoutApprove.click();
	}
	public boolean checkLogin(){
		driver.switchTo().defaultContent();
		if(runMobile)
		{
			waitUntilAppears(By.xpath("//button//div[@class='menuIndicator_t7nb5l']"));
			clickWithJS(dashBoardMobile);
			waitUntilAppears(logoutBtn);
			return isElementPresent(By.xpath("//a[@href='/logout']"));
		
		}
		else
		{
			waitUntilAppears(By.xpath("//a[@href='/dashboard']"));
			return isElementPresent(By.xpath("//a[@href='/dashboard']"));
		}
    }
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("email-login-email"));
	}
}